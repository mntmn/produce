#include <stdio.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <string.h>

#define MIDI_NOTE_ON		0x90
#define MIDI_NOTE_OFF		0x80

jack_port_t* output_port;

jack_client_t	*jack_client = NULL;

struct MidiMessage {
  jack_nframes_t time;
  int len;
  unsigned char data[3];
};

static int toggle = 1;

static int note_queued = 0;
static int note_queued_on = 0;

void* port_buffer;

void queue_midi_note(int note_on) {
  note_queued = 1;
  note_queued_on = note_on;
}

void send_midi(int note_on) {
  unsigned char  *buffer;
  jack_nframes_t	last_frame_time;
  struct MidiMessage ev;

  char channel = 1;

  ev.len = 3;

  if (note_on) {
    ev.data[0] = MIDI_NOTE_ON + channel;
  } else {
    ev.data[0] = MIDI_NOTE_OFF + channel;
  }
  ev.data[1] = 48; // c3
  ev.data[2] = 127; // velocity

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

  printf("midi notes sent\n");
}

int process_callback(jack_nframes_t nframes, void *notused)
{
  jack_midi_clear_buffer(port_buffer);

  if (note_queued) {
    send_midi(note_queued_on);
    note_queued = 0;
  }
  
	//process_midi_input(nframes);
	//process_midi_output(nframes);
  //printf("jack midi callback processed: %d\n",nframes);

	return 0;
}


void init_midi() {
	jack_client = jack_client_open("produceMidiClient", JackNullOption, NULL);

	if (jack_client == NULL) {
		printf("MIDI: Could not connect to the JACK server; run jackd first?\n");
		return;
	}

  output_port = jack_port_register(
    jack_client,
    "produceMidiOut",
    JACK_DEFAULT_MIDI_TYPE,
		JackPortIsOutput,
		0);

  printf("JACK MIDI: output port %p\n",output_port);
  
  port_buffer = jack_port_get_buffer(output_port, 1024);
  
	if (jack_set_process_callback(jack_client, process_callback, 0)) {
		printf("JACK MIDI: Could not register JACK process callback.\n");
		exit(1);
	}
  

	if (jack_activate(jack_client)) {
		printf("JACK MIDI: Cannot activate JACK client.\n");
    exit(1);
	}
}

