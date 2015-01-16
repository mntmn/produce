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

extern "C" {
#include <lyd/lyd.h>
#include "minilisp/minilisp.h"
#include "minilisp/reader.h"
}

extern "C" void init_midi();
extern "C" void send_midi();
extern "C" void queue_midi_note(int note_on);

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
  LydProgram* lyd; // compiled instrument
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
  
  Instrument* instr; // direct pointer to instrument

  long _prev_inpoint;
};

struct Track {
  int id;
  track_type_t type;
  string title;
  
  View* view;

  vector<MPRegion*> regions;
};

struct Project {
  vector<Track*> tracks;
  vector<MPRegion*> regions;  
  vector<Instrument*> instruments;
};



static Project active_project;
static int playback_enabled = 0;
#define QUANTUM_NANOSEC 10000L
static int running = 1;

static int wave_handler (Lyd *lyd, const char *wavename, void *user_data)
{
  SNDFILE *infile;
  SF_INFO  sfinfo;

  sfinfo.format = 0;
  if (!(infile = sf_open (wavename, SFM_READ, &sfinfo)))
  {
    float data[10];
    lyd_load_wave(lyd, wavename, 10, 400, data);
    printf ("failed to open file %s\n", wavename);
    sf_perror (NULL);
    return 1;
  }

  {
    float *data = (float*)malloc(sfinfo.frames * sizeof (float));
    sf_read_float (infile, data, sfinfo.frames);
    lyd_load_wave (lyd, wavename, sfinfo.frames, sfinfo.samplerate, data);
    free (data);
    sf_close (infile);
    printf ("loaded %s\n", wavename);
  }
  return 0;
}

static Lyd *lyd = lyd_new();
static long playhead = 0;
// beat = 1/4 bar
// if bar = 2s, then beat = 0.5s; minute = 60*2 beats; 120bpm
static float bpm = 120.0; 
#define TMUL 1000000L

static long loop_start_point = 0;
static long loop_end_point = 1000;

void clear_fired_regions() {
  for (Track* t : active_project.tracks) {
    for (MPRegion* r : t->regions) {
      r->fired = false;
      r->stopped = false;
    }
  }
}

void audio_task()
{
  if (!lyd_audio_init(lyd, "auto"))
  {
    lyd_free (lyd);
    printf ("failed to initialize lyd (audio output)\n");
    return;
  }

  struct timespec tim, tim2;
  tim.tv_sec = 0;
  tim.tv_nsec = QUANTUM_NANOSEC;
  
  struct timespec start_time;
  struct timespec last_time;

  int voice_tag = 1;
  
  clock_gettime(CLOCK_REALTIME, &start_time);
  while (running) {
    last_time.tv_nsec = start_time.tv_nsec;
    clock_gettime(CLOCK_REALTIME, &start_time);
    long delta = start_time.tv_nsec-last_time.tv_nsec;
    float bpm_factor = 240.0/bpm;
    
    if (delta<0) {
      delta = 65000; // FIXME
    }
    //printf("delta: %ld pb: %d\n",delta,playback_enabled);

    if (playback_enabled) {
      //printf("delta: %ld playhead: %ld pb: %d\n",delta,playhead,playback_enabled);
      playhead += delta;
      if (playhead>loop_end_point*bpm_factor*TMUL) {
        playhead = loop_start_point*bpm_factor*TMUL;
        printf("looped to %d\n",playhead);
        
        // looped.
        // clear fired flags of regions
        clear_fired_regions();
      }
      //printf("elaps: %ld\n",elaps);

      for (Track* t : active_project.tracks) {
        for (MPRegion* r : t->regions) {
          long rstart = (long)r->inpoint * TMUL * bpm_factor;
          long rstop  = rstart + (long)r->length * TMUL * bpm_factor;
          Instrument* instr = active_project.instruments[r->instrument_id];
            
          if (!r->fired && playhead>=rstart && playhead<rstop) {
            printf("r: %d rstart: %ld at %ld\n",r->id,rstart,playhead);

            //printf("trigger %d at %d",ri,elaps);
            r->fired = true;
            if (instr->type == I_SAMPLE) {
              LydProgram* prog = instr->lyd;
              LydVoice* voice = lyd_voice_new(lyd, prog, 0, voice_tag++);
              lyd_voice_set_duration(lyd, voice, 0.1);
            } else {
              queue_midi_note(1);
            }
          } else if (r->fired && !r->stopped && playhead>rstop) {
            r->stopped = true;
            if (instr->type == I_MIDI) {
              queue_midi_note(0);
            }
          }
        }
      }
    }
    
    nanosleep(&tim, &tim2);
  }
  
  //lyd_program_free(instrument);
  lyd_free (lyd);
  //return 0;
}

GLV glv_root;
View* tracks_view;
vector<View*> grid_lines;
View* header_view;
View* playhead_view;
View* loop_start_marker;
View* loop_end_marker;
bool transforming = false;

Buttons* toolbar;
NumberDialer* bpm_dialer;

int win_w;
int win_h;
float track_h;
float zoom_x = 0.5;

void update_model_from_ui() {
  Project& p = active_project;
  
  for (Track* t : p.tracks) {
    for (MPRegion* r : t->regions) {
      if (r->view) {
        // move ui updates to model
        r->inpoint = (long)r->view->left()/zoom_x;
        //printf("region %d inpoint: %d\n",r.id,r.inpoint);
      }
    }
  }
}


void zoom_in_x() {
  zoom_x *= 2.0;
}

void zoom_out_x() {
  zoom_x /= 2.0;
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

void toggle_playback() {
  clear_fired_regions();
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

bool on_toolbar_mouseup(View* v, GLV& glv) {
  toggle_playback();
  return false;
}

bool on_root_mouseup(View * v, GLV& glv) {
  //printf("on_root_mouseup\n");
  mouse_state = MS_IDLE;
  //update_model_from_ui();
  return false;
}

bool on_root_mousedown(View * v, GLV& glv) {
  //printf("on_root_mouseup\n");
  //transforming = true;
  deselect_regions();
  return false;
}

bool on_region_mousedown(View * v, GLV& glv) {
  MPRegion* r = view_to_region(v);
  printf("on_region_mousedown: %x\n",r);
  if (r) {
    vector<MPRegion*> regions = selected_regions();
    
    if (!glv.keyboard().shift() && !glv.keyboard().ctrl()) {
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

  //printf("root_mousemove %d\n",mouse_state);
  
  if (mouse_state == MS_MOVING) {
    vector<MPRegion*> regions = selected_regions();
    for (MPRegion* r : regions) {
      r->inpoint = snap_time(r->_prev_inpoint + mouse_dx/zoom_x/bpm_factor);
    }
  }
  return true;
}

bool on_bpm_keyup(View * v, GLV& glv) {
  bpm = bpm_dialer->getValue();
}


bool external_edit_selected_region() {
  vector<MPRegion*> rs = selected_regions();
  MPRegion* r = rs[0];
  Instrument* instr = active_project.instruments[r->instrument_id];
  string cmd = "audacity \""+string(instr->path)+"\"";
  system(cmd.c_str());
}

bool on_root_keydown(View * v, GLV& glv) {
  // TODO: bind to lisp
  
  char k = glv.keyboard().key();
  printf("key: %d\n",k);
  switch (k) {
  case 'e':
    // open region in audio editor
    external_edit_selected_region();
    break;
  case ' ':
    toggle_playback();
    break;
  case '+':
    zoom_in_x();
    break;
  case '-':
    zoom_out_x();
    break;
  case 13:
    // cursor left
    playhead -= 250*TMUL;
    break;
  case 15:
    playhead += 250*TMUL;
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

static int song_bars = 24;

void update_ui() {
  Project& p = active_project;
  float bpm_factor = 240.0/bpm;

  glv_root.disable(DrawBack);
	
  if (!tracks_view) {
    tracks_view = new View(Rect(0,100, win_w, win_h));
    tracks_view->colors().set(Color(0.2,0.4,1,0.8), 0.3);
    tracks_view->disable(DrawBack);
    tracks_view->disable(DrawBorder);

    for (int i=0; i<song_bars; i++) {
      // 1000px = 1  bar
      // 250px = 1/4 bar (beat)
      int x = (int)(250.0*zoom_x*bpm_factor*i);
      View* l = new View(Rect(x,0,1,win_h));
      if (i%4>0) {
        l->cloneStyle().colors().set(Color(1,1,1,0.1), 1.0);
      } else {
        l->cloneStyle().colors().set(Color(1,1,1,0.2), 1.0);
      }
      l->disable(DrawBack);
      grid_lines.push_back(l);
      *tracks_view << l;
    }
    
    glv_root << tracks_view;
  } else {
    int i = 0;
    tracks_view->height(win_h);
    for (View* l : grid_lines) {
      int x = (int)(250.0*zoom_x*bpm_factor*(i++));
      l->left(x);
      l->height(win_h);
    }
  }

  if (!header_view) {
    header_view = new View(Rect(0,0,win_w*2,50));
    header_view->cloneStyle().colors().set(Color(1.0,1.0,1.0,0.3), 0.9);
    toolbar = new Buttons(Rect(0,0,200,50), 4);
    toolbar->cloneStyle().colors().set(Color(1.0,1.0,1.0,0.6), 0.9);
    *header_view << (View*)toolbar;

    bpm_dialer = new NumberDialer(Rect(250,3,6,44), 3, 2, 999.0, 1.0);
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

    toolbar->on(Event::MouseUp, on_toolbar_mouseup);
    bpm_dialer->on(Event::KeyUp, on_bpm_keyup);
    bpm_dialer->on(Event::MouseUp, on_bpm_keyup);
    
    loop_start_marker->on(Event::MouseDown, on_loop_start_init_drag);
    loop_start_marker->on(Event::MouseDrag, on_loop_start_drag);
    loop_end_marker->on(Event::MouseDown, on_loop_end_init_drag);
    loop_end_marker->on(Event::MouseDrag, on_loop_end_drag);
  } else {
    loop_start_marker->left(loop_start_point*zoom_x*bpm_factor);
    loop_end_marker->left(loop_end_point*zoom_x*bpm_factor);
  }

  if (!playhead_view) {
    playhead_view = new View(Rect(0,51,1,win_h));
    playhead_view->cloneStyle().colors().set(Color(1.0,1.0,1.0,0.6), 0.9);

    glv_root << playhead_view;
  } else {
    float bpm_factor = 240.0/bpm;
    playhead_view->left((float)(playhead/TMUL)*zoom_x);
    playhead_view->height(win_h);
  }
  
  int i=0;
  for (Track* t : p.tracks) {
    if (!t->view) {
      t->view = new View(Rect(0,track_h*i,win_w*2,track_h));
      t->view->colors().set(Color(0.2,0.4,1,0.5), 0.5);
      t->view->disable(DrawBack);
      *tracks_view << *t->view;
      printf("track view added: %x\n",t->view);
    }
    
    for (MPRegion* r : t->regions) {
      Rect rect = Rect(r->inpoint*zoom_x*bpm_factor,0,r->length*zoom_x,track_h);
      if (!r->view) {
        r->view = new View(rect);
        r->view->cloneStyle();
        *t->view << *r->view;
        r->view->on(Event::MouseDown, on_region_mousedown);
      } else {
        r->view->set(rect);
        if (r->selected) {
          r->view->colors().set(Color(0.2,0.4,1,1.0), 0.8);
        } else {
          r->view->colors().set(Color(0.2,0.4,1,0.5), 0.5);
        }
      }
    }
    
    i++;
  }

}

Cell* add_instrument(Cell* args, Cell* env) {
  int id = car(args)->value;
  instrument_type_t type = (instrument_type_t)car(cdr(args))->value;
  char* path = (char*)(car(cdr(cdr(args)))->addr);

  char* code = NULL;

  if (type == I_SAMPLE) {
    code = (char*)malloc(strlen(path)+128);
    sprintf(code,"wave('%s') * 0.5",path);
  }
  
  Instrument* i = new Instrument {
    id,
    type,
    path,
    code
  };

  if (code) {
    i->lyd = lyd_compile(lyd, code);

  }
  active_project.instruments.push_back(i);

  printf("add_instrument: %d %s\n",id,path);
  return alloc_nil();
}

Cell* add_audio_track(Cell* args, Cell* env) {
  int id = car(args)->value;
  char* name = (char*)(car(cdr(args))->addr);

  printf("add_audio_track: %d %s\n",id,name);
  
  Track* t = new Track {id, TRACK_AUDIO, name};
  
  active_project.tracks.push_back(t);
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

  printf("add_region: %d\n",id);
  
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
    snprintf(buf,1023,"(instrument %d %d \"%s\")",i->id,i->type,i->path);
    Cell* r_cell = read_string(buf);
    r_list = append(r_cell, r_list);
  }
  return r_list;
}

Cell* lisp_all_tracks(Cell* args, Cell* env) {
  Cell* r_list = alloc_nil();
  for (Track* t : active_project.tracks) {
    char buf[1024];
    snprintf(buf,1023,"(track %d \"%s\")",t->id,t->title.c_str());
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

void init_lisp_funcs() {
  init_lisp();
  
  register_alien_func("instrument",add_instrument);
  register_alien_func("track",add_audio_track);
  register_alien_func("region",add_region);
  
  register_alien_func("all-instruments",lisp_all_instruments);
  register_alien_func("all-tracks",lisp_all_tracks);
  register_alien_func("all-regions",lisp_all_regions);
  
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

extern void x11_stuff_init();
extern void x11_exit_cleanup();

int main(int argc, char **argv) {
  lyd_set_wave_handler(lyd, wave_handler, NULL);
  
  init_lisp_funcs();

  init_midi();
  
  win_w = 1800;
  win_h = 1000;

  x11_stuff_init();
  //get_windowsize_x11(&win_w, &win_h);
  //win_h/=2;
  //win_w-=100;
  
  atexit(x11_exit_cleanup);

  //printf("XdndAware: %d\n",XdndAware);
  
  track_h = 50;

  load_init_file();
  //load_project_file();

  printf("Tracks: %d\n", active_project.tracks.size());

  update_ui();

  glv_root.on(Event::MouseMove, on_root_mousemove);
  glv_root.on(Event::MouseDrag, on_root_mousemove);
  glv_root.on(Event::MouseDown, on_root_mousedown);
  glv_root.on(Event::MouseUp, on_root_mouseup);
  glv_root.on(Event::KeyDown, on_root_keydown);
  glv_root.on(Event::KeyRepeat, on_root_keydown);
  //glv_root.enable(Controllable | HitTest);
  
  glv::Window win(win_w, win_h, "mnt produce", &glv_root);

  std::thread t1(ui_update_task);
  std::thread t2(audio_task);
  
  playback_enabled = 0;

  Application::run();
}

