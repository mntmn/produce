#include <stdio.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <string.h>

#define MIDI_NOTE_ON		0x90
#define MIDI_NOTE_OFF		0x80

#define NUM_MIDI_PORTS  8
#define MAX_MIDI_QUEUE_LEN 64

jack_port_t* output_ports[NUM_MIDI_PORTS];
void* port_buffers[NUM_MIDI_PORTS];

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

static struct MidiEvent event_queue[MAX_MIDI_QUEUE_LEN];
static int events_queued = 0;

void queue_midi_note(int note, int note_on, int port, char channel, int velocity) {
  event_queue[events_queued].note = note;
  event_queue[events_queued].note_on = note_on;
  event_queue[events_queued].port = port;
  event_queue[events_queued].channel = channel;
  event_queue[events_queued].velocity = velocity;
  
  events_queued++;
}

void send_midi(int note, int note_on, int port, char channel, int velocity) {
  unsigned char  *buffer;
  jack_nframes_t	last_frame_time;
  struct MidiMessage ev;

  channel = 0;

  void* port_buffer = port_buffers[port];

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

  //printf("midi notes sent\n");
}

int process_callback(jack_nframes_t nframes, void *notused)
{
  for (int i=0; i<NUM_MIDI_PORTS; i++) {
    jack_midi_clear_buffer(port_buffers[i]);
  }

  if (events_queued) {
    for (int i=0; i<events_queued; i++) {
      send_midi(event_queue[i].note, event_queue[i].note_on, event_queue[i].port, event_queue[i].channel, event_queue[i].velocity);
    }
    events_queued = 0;
  }
  
	//process_midi_input(nframes);
	//process_midi_output(nframes);
  //printf("jack midi callback processed: %d\n",nframes);

	return 0;
}


void init_midi() {
	jack_client = jack_client_open("produce_midi_client", JackNullOption, NULL);

	if (jack_client == NULL) {
		printf("MIDI: Could not connect to the JACK server; run jackd first?\n");
		return;
	}

  for (int i=0; i<NUM_MIDI_PORTS; i++) {
    char buf[64];
    sprintf(buf,"produce_midi_out_%d",i);
    output_ports[i] = jack_port_register(
      jack_client,
      buf,
      JACK_DEFAULT_MIDI_TYPE,
      JackPortIsOutput,
      0);
    printf("JACK MIDI: output port %d %p\n",i,output_ports[i]);
    port_buffers[i] = jack_port_get_buffer(output_ports[i], 1024);
  }
  
	if (jack_set_process_callback(jack_client, process_callback, 0)) {
		printf("JACK MIDI: Could not register JACK process callback.\n");
		exit(1);
	}
  

	if (jack_activate(jack_client)) {
		printf("JACK MIDI: Cannot activate JACK client.\n");
    exit(1);
	}
}

