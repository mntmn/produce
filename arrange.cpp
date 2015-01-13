#include <unistd.h>
#include <stdlib.h>
#include <sndfile.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include <thread>
#include <vector>
#include <fstream>

#include "arrange.h"

extern "C" {
#include <lyd/lyd.h>
#include "minilisp/minilisp.h"
#include "minilisp/reader.h"
}

using namespace std;

struct Note {
  int freq;
  int velo;
  
  View* view;
};

struct Instrument {
  char* code; // instrument source code
  LydProgram* lyd; // compiled instrument
};

struct Region {
  int id;
  int track_id;
  int type;
  long inpoint;
  long length;
  int instrument_id;
  bool selected;
  vector<Note> notes;

  bool fired; // fired in current loop?
  View* view;
  
  Instrument* instr; // direct pointer to instrument
};

struct Track {
  int id;
  track_type_t type;
  string title;
  
  View* view;

  vector<Region> regions;
};

struct Project {
  vector<Track> tracks;
  vector<Region> regions;  
  vector<Instrument> instruments;
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
static long loop_end_point = TMUL*1000;

void clear_fired_regions() {
  for (Track& t : active_project.tracks) {
    for (Region& r : t.regions) {
      r.fired = false;
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
      if (playhead>loop_end_point*bpm_factor) {
        playhead = loop_start_point*bpm_factor;
        printf("looped to %d\n",playhead);
        
        // looped.
        // clear fired flags of regions
        clear_fired_regions();
      }
      //printf("elaps: %ld\n",elaps);

      for (Track& t : active_project.tracks) {
        for (Region& r : t.regions) {
          long rstart = (long)r.inpoint * TMUL * bpm_factor;
          long rstop  = rstart + (long)r.length * TMUL * bpm_factor;
          if (!r.fired && playhead>=rstart && playhead<rstop) {
            printf("r: %d rstart: %ld at %ld\n",r.id,rstart,playhead);
            
            //printf("trigger %d at %d",ri,elaps);
            r.fired = true;
            LydProgram* instr = active_project.instruments[r.instrument_id].lyd;
            //printf("r: %d instr: %d %x\n",r.id,r.sample_id,instr);
            LydVoice* voice = lyd_voice_new(lyd, instr, 0, voice_tag++);
            lyd_voice_set_duration(lyd, voice, 0.1);
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
  
  for (Track& t : p.tracks) {
    for (Region& r : t.regions) {
      if (r.view) {
        // move ui updates to model
        r.inpoint = (long)r.view->left()/zoom_x;
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

void toggle_playback() {
  clear_fired_regions();
  playback_enabled = 1-playback_enabled;
}

bool on_toolbar_mouseup(View* v, GLV& glv) {
  toggle_playback();
  return false;
}

bool on_root_mouseup(View * v, GLV& glv) {
  //printf("on_root_mouseup\n");
  mouse_state = MS_IDLE;
  //update_model_from_ui();
  return true;
}

bool on_root_mousedown(View * v, GLV& glv) {
  //printf("on_root_mouseup\n");
  //transforming = true;
  return false;
}

Region* view_to_region(View* v) {
  for (Track& t : active_project.tracks) {
    for (Region& r : t.regions) {
      if (r.view == v) {
        return &r;
      }
    }
  }
  return NULL;
}

Track* region_to_track(Region* r_needle) {
  for (Track& t : active_project.tracks) {
    for (Region& r : t.regions) {
      if (&r == r_needle) {
        return &t;
      }
    }
  }
  return NULL;
}

vector<Region*> selected_regions() {
  vector<Region*> res;
  for (Track& t : active_project.tracks) {
    for (Region& r : t.regions) {
      if (r.selected) {
        res.push_back(&r);
      }
    }
  }
  printf ("selected: %d\n",res.size());
  return res;
}

bool on_region_mousedown(View * v, GLV& glv) {
  Region* r = view_to_region(v);
  printf("on_region_mousedown: %x\n",r);
  if (r) {
    
    vector<Region*> regions = selected_regions();

    if (!glv.keyboard().shift()) {
      for (Region* r : regions) {
        r->selected = false;
      }
    }

    if (glv.keyboard().ctrl()) {
      // clone region
      for (Region* sr : regions) {
        Track* t = region_to_track(sr);
        if (t) {
          Region dup = *sr;
          dup.view = NULL;
          dup.selected = true;
          t->regions.push_back(dup);
        }
      }
    } else {
      r->selected = true;
    }
    mouse_state = MS_MOVING;
  }
  return true;
}

bool on_loop_start_drag(View* v, GLV& glv) {
  float bpm_factor = 240.0/bpm;
  
  Mouse m = glv.mouse();

  loop_start_point += TMUL*(m.dx()/zoom_x/bpm_factor);
}

bool on_loop_end_drag(View* v, GLV& glv) {
  float bpm_factor = 240.0/bpm;
  
  Mouse m = glv.mouse();

  loop_end_point += TMUL*(m.dx()/zoom_x/bpm_factor);
}

bool on_root_mousemove(View* v, GLV& glv) {
  float bpm_factor = 240.0/bpm;
  
  Mouse m = glv.mouse();

  //printf("root_mousemove %d\n",mouse_state);
  
  if (mouse_state == MS_MOVING) {
    vector<Region*> regions = selected_regions();
    for (Region* r : regions) {
      printf("  moving dx %f\n",m.dx());
      r->inpoint += m.dx()/zoom_x/bpm_factor;
    }
  }
}

bool on_bpm_keyup(View * v, GLV& glv) {
  bpm = bpm_dialer->getValue();
}

bool on_root_keydown(View * v, GLV& glv) {
  // TODO: bind to lisp
  
  char k = glv.keyboard().key();
  printf("key: %d\n",k);
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
  case 13:
    // cursor left
    playhead -= 250*TMUL;
    break;
  case 15:
    playhead += 250*TMUL;
    break;
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
        l->cloneStyle().colors().set(Color(0.3,0.3,0.3,0.5), 1.0);
      } else {
        l->cloneStyle().colors().set(Color(0.5,0.5,0.5,0.75), 1.0);
      }
      l->disable(DrawBack);
      grid_lines.push_back(l);
      *tracks_view << l;
    }
    
    glv_root << tracks_view;
  } else {
    int i = 0;
    for (View* l : grid_lines) {
      int x = (int)(250.0*zoom_x*bpm_factor*(i++));
      l->left(x);
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
    
    loop_start_marker->on(Event::MouseDrag, on_loop_start_drag);
    loop_end_marker->on(Event::MouseDrag, on_loop_end_drag);
  } else {
    loop_start_marker->left(loop_start_point/TMUL*zoom_x*bpm_factor);
    loop_end_marker->left(loop_end_point/TMUL*zoom_x*bpm_factor);
  }

  if (!playhead_view) {
    playhead_view = new View(Rect(0,51,1,win_h));
    playhead_view->cloneStyle().colors().set(Color(1.0,1.0,1.0,0.6), 0.9);

    glv_root << playhead_view;
  } else {
    float bpm_factor = 240.0/bpm;
    playhead_view->left((float)(playhead/TMUL)*zoom_x);
  }
  
  int i=0;
  for (Track& t : p.tracks) {
    if (!t.view) {
      printf("!t.view: %x\n",t.view);
      t.view = new View(Rect(0,track_h*i,win_w*2,track_h));
      t.view->colors().set(Color(0.2,0.4,1,0.5), 0.5);
      t.view->disable(DrawBack);
      *tracks_view << *t.view;
      printf("track view added: %x\n",&t.view);
    }
    
    for (Region& r : t.regions) {
      Rect rect = Rect(r.inpoint*zoom_x*bpm_factor,0,r.length*zoom_x,track_h);
      if (!r.view) {
        r.view = new View(rect);
        r.view->cloneStyle();
        *t.view << *r.view;
        r.view->on(Event::MouseDown, on_region_mousedown);
      } else {
        r.view->set(rect);
        if (r.selected) {
          r.view->colors().set(Color(0.2,0.4,1,1.0), 0.8);
        } else {
          r.view->colors().set(Color(0.2,0.4,1,0.5), 0.5);
        }
      }
    }
    
    i++;
  }

}

Cell* add_instrument(Cell* args, Cell* env) {
  int id = car(args)->value;
  char* code = (char*)(car(cdr(args))->addr);

  Instrument i = {
    code
  };
  
  i.lyd = lyd_compile(lyd, code);
  active_project.instruments.push_back(i);

  printf("add_instrument: %d %s\n",id,code);
  return alloc_nil();
}

Cell* add_audio_track(Cell* args, Cell* env) {
  int id = car(args)->value;
  char* name = (char*)(car(cdr(args))->addr);

  printf("add_audio_track: %d %s\n",id,name);
  
  Track t = {id, TRACK_AUDIO, name};
  
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
  for (Track& t : active_project.tracks) {
    if (t.id == track_id) track = &t;
  }
  if (!track) {
    printf("(region-sample) invalid track id %d\n",id);
    return alloc_nil();
  }

  printf("add_region: %d\n",id);
  
  Region r = {id,
              track_id,
              0,
              inpoint,
              duration,
              sample_id};
  track->regions.push_back(r);
  
  return alloc_nil();
}

void init_lisp_funcs() {
  init_lisp();
  
  register_alien_func("instrument",add_instrument);
  register_alien_func("track",add_audio_track);
  register_alien_func("region",add_region);
}

#define LOAD_BUFFER_SIZE 1024*1024
Cell* load_project_file() {
  Cell* evaled = alloc_nil();
  
  char* buffer = (char*)malloc(LOAD_BUFFER_SIZE);
  memset(buffer,0,LOAD_BUFFER_SIZE);
  
  FILE* f;
  if ((f = fopen("project.l","r"))) {
    int l = fread(buffer, 1, LOAD_BUFFER_SIZE, f);
    fclose(f);

    if (l) {
      printf("[boot file bytes read: %d]\r\n\n",l);
      Cell* expr = read_string(buffer);
      evaled = alloc_clone(eval(expr, get_globals()), 0);

      //eval_project(expr);
    }
  }

  free(buffer);
  return evaled;
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

  lyd_set_wave_handler(lyd, wave_handler, NULL);
  
  init_lisp_funcs();
  
	// Create the Views
  win_w = 1600;
  win_h = 768;
  track_h = 100;

  load_project_file();

  printf("Tracks: %d\n", active_project.tracks.size());

	// Set color styles
	//glv.cloneStyle().colors().set(StyleColor::WhiteOnBlack);
	//tracks_view.colors().set(Color(0.2,0.4,1,0.8), 0.3);
  
	// Disable some of the default View properties
	/*v2.disable(DrawBorder);
	v12.disable(FocusHighlight);
	*/

  update_ui();

  glv_root.on(Event::MouseMove, on_root_mousemove);
  glv_root.on(Event::MouseDrag, on_root_mousemove);
  glv_root.on(Event::MouseDown, on_root_mousedown);
  glv_root.on(Event::MouseUp, on_root_mouseup);
  glv_root.on(Event::KeyDown, on_root_keydown);
  glv_root.on(Event::KeyRepeat, on_root_keydown);
  //glv_root.enable(Controllable | HitTest);
  
	Window win(win_w, win_h, "mnt produce", &glv_root);

  std::thread t1(ui_update_task);
  std::thread t2(audio_task);
  
  playback_enabled = 0;

  Application::run();
}

