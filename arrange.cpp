#include <unistd.h>
#include <stdlib.h>
#include <sndfile.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <thread>
#include <vector>
#include <fstream>

#include "arrange.h"

#include <sndfile.h>

extern "C" {
#include "minilisp/minilisp.h"
#include "minilisp/reader.h"
#include "misc.c"
}

extern void x11_stuff_init();
extern void x11_exit_cleanup();

using namespace std;

enum instrument_type_t {
  I_SAMPLE,
  I_MIDI
};

struct Note {
  int freq;
  int velo;
  
  View* view;
};

struct Instrument {
  int id;
  instrument_type_t type;
  char* path;
  char* code; // instrument source code
  int note;
  int midi_port;
  int midi_channel;
  float* pcm;
  uint32_t pcm_size;
};

struct MPRegion {
  int id;
  int track_id;
  long inpoint;
  long length;
  int instrument_id;
  bool selected;
  vector<Note> notes;

  bool fired; // fired in current loop?
  bool stopped; // note off sent in current loop?
  View* view;

  long _prev_inpoint;
};

struct Track {
  int id;
  track_type_t type;
  string title;
  int r;
  int g;
  int b;
  
  View* view;

  vector<MPRegion*> regions;

  char label[256];
};

struct Project {
  vector<Track*> tracks;
  vector<Instrument*> instruments;
};

static Project active_project;
static int playback_enabled = 0;
static int bounce_enabled = 0;
#define QUANTUM_NANOSEC 10000L
static int running = 1;

#include <stdio.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <string.h>

#define MIDI_NOTE_ON		0x90
#define MIDI_NOTE_OFF		0x80

#define NUM_MIDI_PORTS  8
#define MAX_MIDI_QUEUE_LEN 64

#define NUM_AUDIO_PORTS  16

#define JACK_BUFSIZE 1024

jack_port_t* midi_output_ports[NUM_MIDI_PORTS];
void* midi_port_buffers[NUM_MIDI_PORTS];

jack_port_t* audio_output_ports[NUM_AUDIO_PORTS];
void* audio_port_buffers[NUM_AUDIO_PORTS];

jack_client_t	*jack_client = NULL;

struct MidiMessage {
  jack_nframes_t time;
  int len;
  unsigned char data[3];
};

struct MidiEvent {
  int note;
  int note_on;
  int port;
  int channel;
  int velocity;
};


void send_midi(int note, int note_on, int port, char channel, int velocity) {
  unsigned char  *buffer;
  jack_nframes_t	last_frame_time;
  struct MidiMessage ev;

  channel = 0;

  void* port_buffer = midi_port_buffers[port];

  ev.len = 3;

  printf("send_midi: %d %d %d %d %d\n",note,note_on,port,channel,velocity);
  
  if (note_on) {
    ev.data[0] = MIDI_NOTE_ON + channel;
  } else {
    ev.data[0] = MIDI_NOTE_OFF + channel;
  }
  ev.data[1] = note; // c3
  ev.data[2] = velocity; // velocity

  if (port_buffer == NULL) {
    printf("jack_port_get_buffer failed, cannot send anything.\n");
    return;
  }
  
  buffer = jack_midi_event_reserve(port_buffer, 0, ev.len);
  if (buffer == NULL) {
    printf("jack_midi_event_reserve (1) failed, NOTE ON LOST.\n");
    return;
  }
  memcpy(buffer, ev.data, 3);
}

static long playhead = 0;
// beat = 1/4 bar
// if bar = 2s, then beat = 0.5s; minute = 60*2 beats; 120bpm
static float bpm = 120.0; 
#define TMUL 1000000L

static long loop_start_point = 0;
static long loop_end_point = 1000;

long playhead_samples = 0;

static int playback_clean_up = 0;

int jack_process_callback(jack_nframes_t nframes, void *notused)
{
  for (int i=0; i<NUM_MIDI_PORTS; i++) {
    jack_midi_clear_buffer(midi_port_buffers[i]);
  }

  /*if (events_queued) {
    for (int i=0; i<events_queued; i++) {
      send_midi(event_queue[i].note, event_queue[i].note_on, event_queue[i].port, event_queue[i].channel, event_queue[i].velocity);
    }
    events_queued = 0;
    }*/
  

  //printf("out buffer: %p\n",out);
  // playhead is in nanoseconds (?)

  float bpm_factor = 240.0/bpm;
  long delta = TMUL*((double)nframes/48.0);

  //printf("delta: %ld playback: %d\n",delta,playback_enabled);

  if (playback_enabled) {
    playhead += delta;
    
    if (playhead>loop_end_point*bpm_factor*TMUL) {
      playhead = loop_start_point*bpm_factor*TMUL;
      printf("looped to %d\n",playhead);
        
      // looped.
      // clear fired flags of regions
      playback_clean_up = 1;

      if (bounce_enabled) {
        playback_enabled = 0;
        bounce_enabled = 0;
        printf("-- finished bounce.\n");
      }
    }
    //printf("elaps: %ld\n",elaps);
    playhead_samples = playhead/TMUL*48.0;
    printf("sample pos: %ld\n",playhead_samples);

    int audio_port_idx = 0;
    int ti = 0;

    for (Track* t : active_project.tracks) {
      float* audio_out = NULL;
      
      Instrument* default_instr = active_project.instruments[ti];

      if (default_instr->type == I_SAMPLE) {
        audio_out = (float*)jack_port_get_buffer(audio_output_ports[audio_port_idx], nframes);
        memset(audio_out, 0, nframes*sizeof(float));
        
        audio_port_idx = (audio_port_idx+1)%NUM_AUDIO_PORTS;
      }
      
      for (MPRegion* r : t->regions) {
        long rstart = (long)r->inpoint * TMUL * bpm_factor;
        long rstop  = rstart + ((long)r->length/2 * TMUL * bpm_factor); // FIXME: why is /2 correct?!?

        if (active_project.instruments.size()<=r->instrument_id) {
          printf("error: track has non-existing instrument id %d\n",r->instrument_id);
        } else {
          
          Instrument* instr = active_project.instruments[r->instrument_id];

          if (r->fired && !r->stopped && playhead>=rstop) {
            r->stopped = true;
            if (instr->type == I_MIDI) {
              send_midi(instr->note,0,instr->midi_port,instr->midi_channel,127);
            }
          } else if (!r->fired && playhead>=rstart && playhead<rstop) {
            if (instr->type == I_SAMPLE) {
              //printf("sample voice fired: %s rstart: %ld\n",instr->path,rstart);

              long offset = playhead_samples - (rstart*48)/TMUL;

              if (offset<instr->pcm_size) {
                int size = nframes;
                if (instr->pcm_size<=offset+nframes) {
                  size = instr->pcm_size-offset;
                  r->fired = true;
                  //printf("-- sample ended %s\n",instr->path);
                }
                
                //printf("port: %d offset in sample: %d pcm_size: %d copy_size: %d\n",audio_port_idx-1,offset,instr->pcm_size,size);
                if (audio_out) {
                  memcpy(audio_out, instr->pcm+offset, size*sizeof(float));
                }
              }
            } else {
              r->fired = true;
              send_midi(instr->note,1,instr->midi_port,instr->midi_channel,127);
            }
          }

        }
      } // end region loop

      ti++;
    } // end track loop
  }

  if (playback_clean_up) {
    playback_clean_up = 0;
    int audio_port_idx = 0;
    int ti = 0;
    for (Track* t : active_project.tracks) {
      float* audio_out = NULL;
      
      Instrument* default_instr = active_project.instruments[ti];

      if (default_instr->type == I_SAMPLE) {
        // clear all the buffers
        audio_out = (float*)jack_port_get_buffer(audio_output_ports[audio_port_idx], nframes);
        memset(audio_out, 0, nframes*sizeof(float));
        
        audio_port_idx = (audio_port_idx+1)%NUM_AUDIO_PORTS;
      }
      
      for (MPRegion* r : t->regions) {
        if (r->fired && !r->stopped) {
          Instrument* instr = active_project.instruments[r->instrument_id];
          if (instr->type == I_MIDI) {
            send_midi(instr->note,0,instr->midi_port,instr->midi_channel,127);
          }
        }
        r->fired = false;
        r->stopped = false;
      }

      ti++;
    }
  }
  
  /*if (out[0]) {
    printf("! ");
    for (int i=0; i<128; i++) {
      printf("%04x ",out[i]);
    }
    printf("\n");
    }*/
  
  
	return 0;
}

void init_jack() {
	jack_client = jack_client_open("produce_midi_client", JackNullOption, NULL);

	if (jack_client == NULL) {
		printf("MIDI: Could not connect to the JACK server; run jackd first?\n");
		return;
	}

  for (int i=0; i<NUM_MIDI_PORTS; i++) {
    char buf[64];
    sprintf(buf,"produce_midi_out_%d",i);
    midi_output_ports[i] = jack_port_register(
      jack_client,
      buf,
      JACK_DEFAULT_MIDI_TYPE,
      JackPortIsOutput,
      0);
    printf("JACK MIDI: output port %d %p\n",i,midi_output_ports[i]);
    midi_port_buffers[i] = jack_port_get_buffer(midi_output_ports[i], JACK_BUFSIZE);
  }
  
  for (int i=0; i<NUM_AUDIO_PORTS; i++) {
    char buf[64];
    sprintf(buf,"produce_audio_out_%d",i);
    audio_output_ports[i] = jack_port_register(
      jack_client,
      buf,
      JACK_DEFAULT_AUDIO_TYPE,
      JackPortIsOutput,
      0);
    printf("JACK Audio: output port %d %p\n",i,audio_output_ports[i]);
    //audio_port_buffers[i] = jack_port_get_buffer(audio_output_ports[i], JACK_BUFSIZE);
  }
  
	if (jack_set_process_callback(jack_client, jack_process_callback, 0)) {
		printf("JACK: Could not register JACK process callback.\n");
		exit(1);
	}
  

	if (jack_activate(jack_client)) {
		printf("JACK: Cannot activate JACK client.\n");
    exit(1);
	}
}

static int load_wave_file(Instrument* instr, const char *wavename)
{
  SNDFILE *infile;
  SF_INFO  sfinfo;

  sfinfo.format = 0;
  if (!(infile = sf_open(wavename, SFM_READ, &sfinfo)))
  {
    printf ("-- load_wave_file: failed to open file %s\n");
    sf_perror (NULL);
    return 1;
  }

  float *data = (float*)malloc(sfinfo.frames * sizeof(float));
  sf_read_float(infile, data, sfinfo.frames);

  instr->pcm = data;
  instr->pcm_size = sfinfo.frames;
  
  sf_close(infile);
  printf ("-- load_wave_file: loaded %s %d\n", wavename, instr->pcm_size);
  return 0;
}

GLV glv_root;
View* tracks_view;
View* selection_rect;
vector<View*> grid_lines;
View* header_view;
View* playhead_view;
View* loop_start_marker;
View* loop_end_marker;
bool transforming = false;

Track* selected_track = NULL;

Buttons* toolbar;
NumberDialer* bpm_dialer;
TextView* title_view;

int win_w;
int win_h;
float track_h;
float scroll_x = 500;
float zoom_x = 0.5;

void zoom_in_x() {
  zoom_x *= 2.0;
}

void zoom_out_x() {
  zoom_x /= 2.0;
}

void scroll_left() {
  scroll_x += 1000;
}

void scroll_right() {
  scroll_x -= 1000;
}

enum mouse_state_t {
  MS_IDLE,
  MS_MOVING,
  MS_RESIZING,
  MS_SELECTING
};

mouse_state_t mouse_state = MS_IDLE;
int mouse_dx = 0;
int mouse_dy = 0;
long drag_x1 = 0;
long drag_y1 = 0;
int old_track_dy = 0;
View* hover_track_view = NULL;

void toggle_playback() {
  playback_clean_up = 1;
  playback_enabled = 1-playback_enabled;
}

MPRegion* view_to_region(View* v) {
  for (Track* t : active_project.tracks) {
    for (MPRegion* r : t->regions) {
      if (r->view == v) {
        return r;
      }
    }
  }
  return NULL;
}

Track* view_to_track(View* v) {
  for (Track* t : active_project.tracks) {
    if (t->view == v) {
      return t;
    }
  }
  return NULL;
}

Track* region_to_track(MPRegion* r_needle) {
  for (Track* t : active_project.tracks) {
    for (MPRegion* r : t->regions) {
      if (r == r_needle) {
        return t;
      }
    }
  }
  return NULL;
}

vector<MPRegion*> selected_regions() {
  vector<MPRegion*> res;
  for (Track* t : active_project.tracks) {
    for (MPRegion* r : t->regions) {
      if (r->selected) {
        res.push_back(r);
      }
    }
  }
  return res;
}

void deselect_regions() {
  vector<MPRegion*> regions = selected_regions();

  for (MPRegion* r : regions) {
    r->selected = false;
  }
}

void delete_selected_regions() {
  vector<MPRegion*> regions = selected_regions();

  for (MPRegion* r : regions) {
    Track* t = region_to_track(r);
    if (t) {
      vector<MPRegion*>& v = t->regions;
      r->view->remove();
      v.erase(remove(begin(v), end(v), r), end(v));
    }
  }
}

void select_regions_in_rect(Rect& rect) {
  for (Track* t : active_project.tracks) {
    for (MPRegion* r : t->regions) {
      if (r->view) {
        Rect shifted = Rect(*(r->view));
        shifted.posAdd(t->view->left(), t->view->top());
      
        if (shifted.intersects(rect)) {
          //printf("intersects: %f %f %f %f\n",r->view->left(),r->view->top(),r->view->width(),r->view->height());
          r->selected = true;
        }
      }
    }
  }
}

bool on_toolbar_mouseup(View* v, GLV& glv) {
  toggle_playback();
  return false;
}

bool on_root_mouseup(View * v, GLV& glv) {
  //printf("on_root_mouseup\n");
  
  if (mouse_state == MS_SELECTING) {
    select_regions_in_rect(*selection_rect);
    selection_rect->width(0);
    selection_rect->height(0);
  }
  mouse_state = MS_IDLE;

  return false;
}

bool on_root_mousedown(View * v, GLV& glv) {
  //printf("on_root_mouseup\n");
  //transforming = true;
  deselect_regions();
  return false;
}

bool on_region_mousedown(View * v, GLV& glv) {

  mouse_state = MS_MOVING;
  
  MPRegion* r = view_to_region(v);
  printf("on_region_mousedown: %x\n",r);
  if (r) {
    vector<MPRegion*> regions = selected_regions();
    
    if (!glv.keyboard().shift() && !glv.keyboard().ctrl() && regions.size()<2) {
      for (MPRegion* r : regions) {
        r->selected = false;
      }
    }

    r->selected = true;
      
    if (glv.keyboard().ctrl()) {
      // clone region
      for (MPRegion* sr : regions) {
        Track* t = region_to_track(sr);
        if (t) {
          MPRegion* dup = new MPRegion;
          memcpy(dup, sr, sizeof(MPRegion));
          dup->view = NULL;
          dup->selected = true;
          t->regions.push_back(dup);
          sr->selected = false;
        }
      }
    }
    
    mouse_state = MS_MOVING;
    mouse_dx = 0;
    mouse_dy = 0;
    old_track_dy = 0;
    drag_x1 = glv.mouse().x();
    drag_y1 = glv.mouse().y();

    for (MPRegion* sr : selected_regions()) {
      sr->_prev_inpoint = sr->inpoint; // save state when we started dragging
    }
  }
  return false;
}

long snap_time(long p) {
  long snap = 1000/8;
  long thresh = 20;
  
  long p1=p-(p%snap);
  long p2=snap+p-(p%snap);

  //printf("p: %ld p1: %ld\n",p,p1);

  if (abs(p-p1) < thresh) return p1;
  if (abs(p-p2) < thresh) return p2;
  
  return p;
}

bool on_loop_start_init_drag(View* v, GLV& glv) {
  drag_x1 = loop_start_point;
  drag_y1 = 0;
  mouse_dx = 0;
  return true;
}

bool on_loop_start_drag(View* v, GLV& glv) {
  float bpm_factor = 240.0/bpm;
  
  Mouse m = glv.mouse();
  mouse_dx += m.dx();
  
  loop_start_point = snap_time(drag_x1 + (mouse_dx/zoom_x/bpm_factor));
  return false;
}

bool on_loop_end_init_drag(View* v, GLV& glv) {
  drag_x1 = loop_end_point;
  drag_y1 = 0;
  mouse_dx = 0;
  return true;
}

bool on_loop_end_drag(View* v, GLV& glv) {
  float bpm_factor = 240.0/bpm;
  
  Mouse m = glv.mouse();
  mouse_dx += m.dx();

  loop_end_point = snap_time(drag_x1 + (mouse_dx/zoom_x/bpm_factor));
  return false;
}

bool on_root_mousemove(View* v, GLV& glv) {
  float bpm_factor = 240.0/bpm;
  
  Mouse m = glv.mouse();
  mouse_dx += m.dx();
  mouse_dy += m.dy();
  
  int track_dy = round((float)mouse_dy/(float)track_h);

  //printf("root_mousemove %d\n",mouse_state);
  
  if (mouse_state == MS_MOVING) {
    int track_ddy = track_dy-old_track_dy;
      
    vector<MPRegion*> regions = selected_regions();
    for (MPRegion* r : regions) {
      r->inpoint = snap_time(r->_prev_inpoint + mouse_dx/zoom_x/bpm_factor);

      // allow cross-track moving only inside bounds
      if ((r->track_id+track_ddy)<0 || (r->track_id+track_ddy)>active_project.tracks.size()-1) {
        track_ddy = 0;
      }
    }

    if (track_ddy!=0) {
      printf("move across tracks %d\n",track_ddy);
      vector<MPRegion*> regions = selected_regions();
      
      for (MPRegion* r : regions) {
        Track* t = region_to_track(r);
        
        vector<MPRegion*>& v = t->regions;
        v.erase(remove(begin(v), end(v), r), end(v));

        int new_id = t->id + track_ddy;
        
        Track* new_track = active_project.tracks[new_id];
        /*MPRegion* new_r = new MPRegion {r->id,
                                new_id,
                                r->inpoint,
                                r->length,
                                new_id};*/
        //new_r->_prev_inpoint = r->_prev_inpoint;
        //new_r->selected = true;
        // recycle the view
        //new_r->view = r->view;
        //r->view = NULL;
        r->track_id = new_id;
        r->instrument_id = new_id;
        *new_track->view << *r->view;
        new_track->regions.push_back(r);
      }
      
      old_track_dy = track_dy;
    }
  }
  return true;
}

bool on_bpm_keyup(View * v, GLV& glv) {
  bpm = bpm_dialer->getValue();
}

bool on_selection_rect_drag(View* v, GLV& glv) {
  if (mouse_state == MS_MOVING) return true;
  
  float dx = glv.mouse().dx();
  float dy = glv.mouse().dy();

  mouse_dx += dx;
  mouse_dy += dy;

  mouse_state = MS_SELECTING;
  
  float x,y,w=mouse_dx,h=mouse_dy;
  
  if (w<0) {
    x = drag_x1 + w;
    w = -w;
  } else {
    x = drag_x1;
    w = w;
  }
  
  if (h<0) {
    y = drag_y1 + h;
    h = -h;
  } else {
    y = drag_y1;
    h = h;
  }
  
  selection_rect->set(x,y,w,h);
  
  //printf("drag %f %f %f %f\n",x,y,w,h);

  return false;
}

Cell* lisp_add_region_at_mouse(Cell* args, Cell* env) {
  float bpm_factor = 240.0/bpm;
  space_t x = glv_root.mouse().x() - scroll_x;
  space_t y = glv_root.mouse().y();
  
  hover_track_view = glv_root.findTarget(x, y);
  printf("hover_track_view: %x\n",hover_track_view);
  if (!hover_track_view) return alloc_nil();

  // create note
  Track* t = view_to_track(hover_track_view);
  if (t) {
    int inpoint = snap_time(x/zoom_x/bpm_factor);
    int duration = 50;
    MPRegion* r = new MPRegion {1,
                                t->id,
                                inpoint,
                                duration,
                                t->id};
    t->regions.push_back(r);
  }

  return alloc_nil();
}

bool on_track_mousedown(View* v, GLV& glv) {
  mouse_dx = 0;
  mouse_dy = 0;
  float x = glv.mouse().x();
  float y = glv.mouse().y();
  drag_x1 = x - tracks_view->left();
  drag_y1 = y - tracks_view->top();
  
  selection_rect->left(drag_x1);
  selection_rect->top(drag_y1);
  selection_rect->width(1);
  selection_rect->height(1);

  //printf("track mousedown %d %d\n",drag_x1,drag_y1);

  hover_track_view = glv_root.findTarget(x, y);
  selected_track = view_to_track(hover_track_view);
  if (selected_track) {
    //printf("selected_track: %p (id: %d)\n",selected_track,selected_track->id);
  }
  
  if (!glv.keyboard().shift()) {
    deselect_regions();
  }

  return false;
}

Cell* external_edit_selected_region(Cell* args, Cell* env) {
  vector<MPRegion*> rs = selected_regions();
  MPRegion* r = rs[0];
  Instrument* instr = active_project.instruments[r->instrument_id];
  string cmd = "mhwaveedit --driver jack \""+string(instr->path)+"\"";
  system(cmd.c_str());

  // TODO: reload audio

  return alloc_nil();
}

bool on_root_keydown(View * v, GLV& glv) {
  char k = glv.keyboard().key();
  //printf("key: %d %d %d\n",k,glv.keyboard().ctrl(),glv.keyboard().alt());

  // FIXME
  
  switch (k) {
  case ' ':
    toggle_playback();
    break;
  case '+':
    zoom_in_x();
    break;
  case '-':
    zoom_out_x();
    break;
  case ',':
    playhead -= 250*TMUL;
    break;
  case '.':
    playhead += 250*TMUL;
    break;
  case 13:
    // cursor left
    scroll_left();
    break;
  case 15:
    scroll_right();
    break;
  case 8: // backspace
    delete_selected_regions();
    break;
  default: {
    char buf[1024];
    sprintf(buf,"(handle-key \"%c\")",k);
    eval(read_string(buf), get_globals());
  }
  }
}

static int song_bars = 4*100;

char* make_track_label(Track* t) {
  //printf("-- make_track_label %d\n",t->id);
  char tmp[128];
  if (t->type == TRACK_MIDI) {
    snprintf(t->label, 255, "%02d MIDI %s", t->id, t->title.c_str());
  } else {
    const char* ptr = t->title.c_str();
    while (strstr(ptr, "/")) {
      // find last slash
      ptr = strstr(ptr, "/") + 1;
    }
    if (!ptr) ptr = t->title.c_str();
    strncpy(tmp,ptr,16);
    tmp[16] = 0;
    
    snprintf(t->label, 255, "%02d %s", t->id, tmp);
  }
  //printf("track %d label: %s\n",t->id,t->label);
  return t->label;
}

void update_ui() {
  Project& p = active_project;
  float bpm_factor = 240.0/bpm;

  win_w = 2560;
  win_h = 1600;

  glv_root.disable(DrawBack);
	
  if (!tracks_view) {
    tracks_view = new View(Rect(0,100, win_w, win_h));
    tracks_view->colors().set(Color(0.2,0.4,1,0.8), 1.0);
    tracks_view->disable(DrawBack);
    tracks_view->disable(DrawBorder);

    for (int i=0; i<song_bars; i++) {
      // 1000px = 1  bar
      // 250px = 1/4 bar (beat)
      int x = (int)(250.0*zoom_x*bpm_factor*i);
      View* l = new View(Rect(x,0,1,win_h));
      if (i%4>0) {
        l->cloneStyle().colors().set(Color(0.9,0.9,1,0.08), 1.0);
      } else {
        l->cloneStyle().colors().set(Color(0.9,0.9,1,0.12), 1.0);
      }
      l->disable(DrawBack);
      grid_lines.push_back(l);
      *tracks_view << l;
    }

    selection_rect = new View(Rect(0,0,0,0));
    selection_rect->cloneStyle().colors().set(Color(1,1,1,0.3), 0.3);
    
    tracks_view->on(Event::MouseDown, on_track_mousedown);
    tracks_view->on(Event::MouseDrag, on_selection_rect_drag);

    *tracks_view << selection_rect;
    
    glv_root << tracks_view;
  } else {
    int i = 0;
    tracks_view->height(win_h);
    for (View* l : grid_lines) {
      int x = (int)(scroll_x + 250.0*zoom_x*bpm_factor*(i++));
      l->left(x);
      l->height(win_h);
    }
  }

  if (!header_view) {
    header_view = new View(Rect(0,0,win_w*2,50));
    header_view->cloneStyle().colors().set(Color(1.0,1.0,1.0,0.5), 0.9);
    /*toolbar = new Buttons(Rect(0,0,200,50), 4);
    toolbar->cloneStyle().colors().set(Color(1.0,1.0,1.0,0.6), 0.9);
    *header_view << (View*)toolbar;*/

    title_view = new TextView(Rect(150,10,340,30), 19.0);
    *header_view << (View*)title_view;
    title_view->setValue("project3.l");
    
    title_view->disable(DrawBorder);
    header_view->disable(DrawBorder);

    bpm_dialer = new NumberDialer(Rect(5,5,50,40), 3, 2, 999.0, 1.0);
    bpm_dialer->disable(DrawBorder);
    bpm_dialer->disable(DrawBack);
    *header_view << (View*)bpm_dialer;

    bpm_dialer->setValue(bpm);

    loop_end_marker = new View(Rect(0,75,25,25));
    loop_end_marker->cloneStyle().colors().set(Color(1.0,0.7,0.9,0.6), 0.9);
    glv_root << loop_end_marker;

    loop_start_marker = new View(Rect(0,75,25,25));
    loop_start_marker->cloneStyle().colors().set(Color(1.0,1.0,1.0,0.6), 0.9);
    glv_root << loop_start_marker;

    glv_root << header_view;

    //toolbar->on(Event::MouseUp, on_toolbar_mouseup);
    bpm_dialer->on(Event::KeyUp, on_bpm_keyup);
    bpm_dialer->on(Event::MouseUp, on_bpm_keyup);
    
    loop_start_marker->on(Event::MouseDown, on_loop_start_init_drag);
    loop_start_marker->on(Event::MouseDrag, on_loop_start_drag);
    loop_end_marker->on(Event::MouseDown, on_loop_end_init_drag);
    loop_end_marker->on(Event::MouseDrag, on_loop_end_drag);
  } else {
    loop_start_marker->left(scroll_x + loop_start_point*zoom_x*bpm_factor);
    loop_end_marker->left(scroll_x + loop_end_point*zoom_x*bpm_factor);
  }

  if (!playhead_view) {
    playhead_view = new View(Rect(0,51,1,win_h));
    playhead_view->cloneStyle().colors().set(Color(1.0,1.0,1.0,0.6), 0.9);

    glv_root << playhead_view;
  } else {
    float bpm_factor = 240.0/bpm;
    playhead_view->left(scroll_x + (float)(playhead/TMUL)*zoom_x);
    playhead_view->height(win_h);
  }
  
  int i=0;
  for (Track* t : p.tracks) {
    if (!t->view) {
      t->view = new View(Rect(0,track_h*i,win_w*2,track_h));

      // 0.2,0.4,1
      
      t->view->cloneStyle().colors().set(Color((float)t->r/10.0,(float)t->g/10.0,(float)t->b/10.0,0.1), 0.1);
      t->view->disable(DrawBack);

      *tracks_view << *t->view;

      Label* l = new Label(make_track_label(t), false);
      l->size(track_h/2);
      l->set(Rect(5,5,100,track_h));
      *t->view << *l;
      
      //printf("track view added: %x\n",t->view);
    } else {
      if (selected_track == t) {
        t->view->enable(DrawBack);
      } else {
        t->view->disable(DrawBack);
      }
    }
    
    for (MPRegion* r : t->regions) {
      Rect rect = Rect(scroll_x + r->inpoint*zoom_x*bpm_factor,0,r->length*zoom_x,track_h);
      if (!r->view) {
        r->view = new View(rect);
        r->view->cloneStyle();
        *t->view << *r->view;
        r->view->on(Event::MouseDown, on_region_mousedown);
      }
      
      r->view->set(rect);
      if (r->selected) {
        r->view->colors().set(Color((float)t->r/10.0,(float)t->g/10.0,(float)t->b/10.0, 0.9));
        r->view->enable(DrawBorder);
      } else {
        r->view->colors().set(Color((float)t->r/10.0,(float)t->g/10.0,(float)t->b/10.0, 0.7));
        r->view->disable(DrawBorder);
      }
    }
    
    i++;
  }
}

Cell* add_instrument(Cell* args, Cell* env) {
  int id = car(args)->value;
  instrument_type_t type = (instrument_type_t)car(cdr(args))->value;
  char* path = (char*)(car(cdr(cdr(args)))->addr);
  int note = 0;
  int port = 0;
  int channel = 0;
  
  char* code = NULL;

  if (type == I_SAMPLE) {
    
  } else {
    path = "";
    note = (car(cdr(cdr(args)))->value);
    port = (car(cdr(cdr(cdr(args))))->value);
  }
  
  Instrument* i = new Instrument {
    id,
    type,
    path,
    code,
    note,
    port,
    channel
  };

  if (type == I_SAMPLE) {
    load_wave_file(i, i->path);
  }
  
  active_project.instruments.push_back(i);

  printf("add_instrument: %d %s\n",id,path);
  return alloc_nil();
}

Cell* set_regions_length(Cell* args, Cell* env) {
  int d = car(args)->value;

  vector<MPRegion*> rs = selected_regions();

  for (MPRegion* r : rs) {
    r->length = d;
  }
  return car(args);
}

Cell* add_audio_track(Cell* args, Cell* env) {
  //int id = car(args)->value;
  int id = active_project.tracks.size();
  
  char* name = (char*)(car(cdr(args))->addr);
  int r = 5;
  int g = 5;
  int b = 5;

  if (car(cdr(cdr(args)))) {
    r = (car(cdr(cdr(args))))->value;
    g = (car(cdr(cdr(cdr(args)))))->value;
    b = (car(cdr(cdr(cdr(cdr(args))))))->value;
  }

  //printf("add_audio_track: %d %s\n",id,name);
  
  Track* t = new Track {id, TRACK_AUDIO, name, r, g, b};
  
  active_project.tracks.push_back(t);
  return alloc_nil();
}

Cell* delete_selected_tracks(Cell* args, Cell* env) {

  deselect_regions();
  if (selected_track) {
    for (MPRegion* r : selected_track->regions) {
      r->selected = true;
    }
    delete_selected_regions();

    vector<Track*>& v = active_project.tracks;

    printf("size before remove: %d\n",active_project.tracks.size());
    
    v.erase(remove(begin(v), end(v), selected_track), end(v));
    selected_track->view->remove();

    // remove track instrument
    vector<Instrument*>& iv = active_project.instruments;
    Instrument* selected_instr = iv[selected_track->id];
    iv.erase(remove(begin(iv), end(iv), selected_instr), end(iv));
    
    printf("size after remove: %d\n",active_project.tracks.size());

    // move all subsequent tracks up (delete their views) and decrement their ids

    printf("deleted track id: %d\n",selected_track->id);

    for (int i=selected_track->id; i<active_project.tracks.size(); i++) {
      Track* t = active_project.tracks[i];
      printf("%d moving up track %p (id: %d)\n",i,t,t->id);
      t->id--;

      for (MPRegion* r : t->regions) {
        r->track_id--;
        r->instrument_id--;
        r->view->remove();
        r->view = NULL;
      }
      t->view->remove(); // FIXME: dealloc view
      t->view = NULL;
    }
  }

  selected_track = NULL;
}

Cell* add_midi_octave(Cell* args, Cell* env) {
  int octave = 3;
  int port = 0;
  int channel = 1;

  if (car(args) && car(args)->tag == TAG_INT) {
    octave = car(args)->value;

    args = cdr(args);
    if (car(args) && car(args)->tag == TAG_INT) {
      port = car(args)->value;
    }
  }
  
  int note = octave*12;
  
  for (int i=0; i<12; i++) {
    int r = 0;
    int g = 7;
    int b = (2+2*port)%10;
    
    char name[64];
    sprintf(name,"M%d N%d",channel,note);
    
    int id = active_project.tracks.size();
    
    Track* t = new Track {id, TRACK_MIDI, name, r, g, b};
    active_project.tracks.push_back(t);

    // make instrument

    Instrument* instr = new Instrument {id, I_MIDI, name, "", note, port, channel};
    active_project.instruments.push_back(instr);
    
    note++;
  }
  return alloc_nil();
}

Cell* bounce_loop(Cell* args, Cell* env) {
  float bpm_factor = 240.0/bpm;
    
  playback_enabled = 0;
  playback_clean_up = 1;
  
  char buf[1024];
  // 2 seconds padding
  int seconds = 2+((loop_end_point-loop_start_point)/250)/bpm*60;
  
  sprintf(buf,"jack_capture -z 2 -d %d &", seconds);
  printf("bouncing using: %s\n",buf);

  system(buf);

  sleep(1);

  // 1 second front padding
  bounce_enabled = 1;
  playhead = loop_start_point*bpm_factor*TMUL;
  playback_enabled = 1;
}

Cell* lisp_eval_dialog(Cell* args, Cell* env) {
  char* expr = get_command_output("zenity --entry");

  if (expr && strlen(expr)>0) {
    return eval(read_string(expr), get_globals());
  }
  return alloc_nil();
}

Cell* lisp_err(char* msg) {
  printf("%s\n",msg);
  return alloc_nil();
}

Cell* add_region(Cell* args, Cell* env) {
  /*
  int id;
  int track_id;
  int type;
  long inpoint;
  long length;
  int sample_id;
  vector<Note> notes;
  */

  if (!car(args) || car(args)->tag!=TAG_INT) return lisp_err("(region-sample) invalid param #0 (id)");
  int id = car(args)->value;

  args=cdr(args);
  if (!car(args) || car(args)->tag!=TAG_INT) return lisp_err("(region-sample) invalid param #1 (track_id)");  
  int track_id = car(args)->value;

  args=cdr(args);
  if (!car(args) || car(args)->tag!=TAG_INT) return lisp_err("(region-sample) invalid param #2 (sample_id)");  
  int sample_id = car(args)->value;
                      
  args=cdr(args);
  if (!car(args) || car(args)->tag!=TAG_INT) return lisp_err("(region-sample) invalid param #3 (inpoint)");  
  int inpoint = car(args)->value;
 
  args=cdr(args);
  if (!car(args) || car(args)->tag!=TAG_INT) return lisp_err("(region-sample) invalid param #4 (duration)");  
  int duration = car(args)->value;

  Track* track;
  for (Track* t : active_project.tracks) {
    if (t->id == track_id) track = t;
  }
  if (!track) {
    printf("(region-sample) invalid track id %d\n",id);
    return alloc_nil();
  }

  // FIXME: region ids are all 1
  //printf("add_region: %d\n",id);
  
  MPRegion* r = new MPRegion {id,
              track_id,
              inpoint,
              duration,
              sample_id};
  track->regions.push_back(r);
  
  return alloc_nil();
}

Cell* lisp_dump(Cell* expr, Cell* env) {
  char buf[1024];
  lisp_write(car(expr), buf, 1024);
  printf("%s\n",buf);
  return alloc_nil();
}

Cell* lisp_all_instruments(Cell* args, Cell* env) {
  Cell* r_list = alloc_nil();
  for (Instrument* i : active_project.instruments) {
    char buf[1024];
    if (i->type == I_SAMPLE) {
      snprintf(buf,1023,"(instrument %d %d \"%s\")",i->id,i->type,i->path);
    } else {
      snprintf(buf,1023,"(instrument %d %d %d %d %d)",i->id,i->type,i->note,i->midi_port,i->midi_channel);
    }
    Cell* r_cell = read_string(buf);
    r_list = append(r_cell, r_list);
  }
  return r_list;
}

Cell* lisp_all_tracks(Cell* args, Cell* env) {
  Cell* r_list = alloc_nil();
  for (Track* t : active_project.tracks) {
    char buf[1024];
    snprintf(buf,1023,"(track %d \"%s\" %d %d %d)",t->id,t->title.c_str(),t->r,t->g,t->b);
    Cell* r_cell = read_string(buf);
    r_list = append(r_cell, r_list);
  }
  return r_list;
}

Cell* lisp_all_regions(Cell* args, Cell* env) {
  Cell* r_list = alloc_nil();
  for (Track* t : active_project.tracks) {
    for (MPRegion* r : t->regions) {
      char buf[1024];
      snprintf(buf,1023,"(region %d %d %d %d %d)",r->id,r->track_id,r->instrument_id,r->inpoint,r->length);
      Cell* r_cell = read_string(buf);
      r_list = append(r_cell, r_list);
    }
  }
  return r_list;
}

Cell* lisp_save_project(Cell* args, Cell* env) {
  char* path = (char*)(car(args)->addr);
  FILE* f = fopen(path,"wb");

  char buf[1024];

  printf("writing project to %s\n",path);
  
  if (f) {
    sprintf(buf,"(let (project-version 1) \n");
    fwrite(buf, 1, strlen(buf), f);
  
    for (Instrument* i : active_project.instruments) {
      if (i->type == I_SAMPLE) {
        sprintf(buf,"(instrument %d %d \"%s\")\n",i->id,i->type,i->path);
      } else {
        sprintf(buf,"(instrument %d %d %d %d %d)\n",i->id,i->type,i->note,i->midi_port,i->midi_channel);
      }
      fwrite(buf, 1, strlen(buf), f);
    }
    for (Track* t : active_project.tracks) {
      sprintf(buf, "\n(track %d \"%s\" %d %d %d)\n",t->id,t->title.c_str(),t->r,t->g,t->b);
      fwrite(buf, 1, strlen(buf), f);
      
      for (MPRegion* r : t->regions) {
        sprintf(buf,"(region %d %d %d %d %d)\n",r->id,r->track_id,r->instrument_id,r->inpoint,r->length);
        fwrite(buf, 1, strlen(buf), f);
      }
    }
  
    sprintf(buf,")\n");
    fwrite(buf, 1, strlen(buf), f);
    fclose(f);
  }
}

void file_dropped_callback(char* uri_raw) {
  printf("file_dropped_callback: %s\n",uri_raw);

  char* extension = 0;

  char uri[1024];
  urldecode2(uri, uri_raw);

  //  && (extension = strstr(uri,".wav"))
  
  if (strstr(uri,"file:///")) {
    //extension[4] = 0;
    char* path = uri+7;
    uri[strlen(uri)-2]=0; // chop LF

    printf("audio file dropped: [%s]\n",path);

    char converted_path[1024];
    sprintf(converted_path, "%s.conv.wav", path);

    char buf2[2048];
    sprintf(buf2,"sox \"%s\" -r 48000 -b 16 -c 1 \"%s\"",path,converted_path);
    printf("converting WAV: [%s]\n",buf2);
    system(buf2);

    int iid = active_project.tracks.size();

    char buf[1024];
    snprintf(buf,1023,"(instrument %d 0 \"%s\")\0",iid,converted_path);

    printf("instr --> %s\n",buf);
    
    eval(read_string(buf), get_globals());

    snprintf(buf,1023,"(track %d \"%s\" %d %d %d)\0",iid,converted_path,4+(iid%3),4+(iid%3),7+iid%4);
    
    printf("track --> %s\n",buf);
    
    eval(read_string(buf), get_globals());
  }
}

Cell* lisp_project_path(Cell* args, Cell* env) {
  Cell* c = alloc_string();
  c->addr = (char*)title_view->getValue().c_str();
  c->size = strlen((char*)c->addr);
  printf("project path: %s\n",c->addr);
  return c;
}

Cell* lisp_clear_project(Cell* args, Cell* env) {
  while (active_project.tracks.size()) {
    selected_track = active_project.tracks[active_project.tracks.size()-1];
    delete_selected_tracks(NULL, env);
  }
  return alloc_nil();
}

void init_lisp_funcs() {
  init_lisp();
  
  register_alien_func("instrument",add_instrument);
  register_alien_func("track",add_audio_track);
  register_alien_func("region",add_region);
  
  register_alien_func("project-path",lisp_project_path);
  register_alien_func("all-instruments",lisp_all_instruments);
  register_alien_func("all-tracks",lisp_all_tracks);
  register_alien_func("all-regions",lisp_all_regions);

  register_alien_func("add-region-at-mouse",lisp_add_region_at_mouse);
  register_alien_func("edit-region-external",external_edit_selected_region);
  
  register_alien_func("octave",add_midi_octave);
  
  register_alien_func("set-regions-length",set_regions_length);

  register_alien_func("delete-selected-tracks",delete_selected_tracks);
  
  register_alien_func("bounce-loop",bounce_loop);
  register_alien_func("interactive-eval",lisp_eval_dialog);
  
  register_alien_func("project-save",lisp_save_project);
  register_alien_func("project-clear",lisp_clear_project);
  
  register_alien_func("print",lisp_dump);
}

#define LOAD_BUFFER_SIZE 1024*1024
Cell* eval_lisp_file(char* filename) {
  Cell* evaled = alloc_nil();
  
  char* buffer = (char*)malloc(LOAD_BUFFER_SIZE);
  memset(buffer,0,LOAD_BUFFER_SIZE);
  
  FILE* f;
  if ((f = fopen(filename,"r"))) {
    int l = fread(buffer, 1, LOAD_BUFFER_SIZE, f);
    fclose(f);

    if (l) {
      Cell* expr = read_string(buffer);
      evaled = alloc_clone(eval(expr, get_globals()), 0);
    }
  }

  free(buffer);
  return evaled;
}

Cell* load_init_file() {
  return eval_lisp_file("init.l");
}

Cell* load_project_file() {
  return eval_lisp_file("project.l");
}

void ui_update_task() {
  struct timespec tim, tim2;
  tim.tv_sec = 0;
  tim.tv_nsec = 25*1000000L;

  while (running) {
    update_ui();
    nanosleep(&tim, &tim2);
  }
}

int main(int argc, char **argv) {
  init_lisp_funcs();

  init_jack();
  
  win_w = 1800;
  win_h = 1000;

  //get_windowsize_x11(&win_w, &win_h);
  //win_h/=2;
  //win_w-=100;
  
  atexit(x11_exit_cleanup);

  //printf("XdndAware: %d\n",XdndAware);
  
  track_h = 30;

  load_init_file();
  //load_project_file();

  update_ui();

  glv_root.on(Event::MouseMove, on_root_mousemove);
  glv_root.on(Event::MouseDrag, on_root_mousemove);
  glv_root.on(Event::MouseDown, on_root_mousedown);
  glv_root.on(Event::MouseUp, on_root_mouseup);
  glv_root.on(Event::KeyDown, on_root_keydown);
  glv_root.on(Event::KeyRepeat, on_root_keydown);
  //glv_root.enable(Controllable | HitTest);
  
  glv::Window win(win_w, win_h, "mnt produce 0.1.0", &glv_root);
  
  x11_stuff_init();

  std::thread t1(ui_update_task);
  
  playback_enabled = 0;

  Application::run();

}

