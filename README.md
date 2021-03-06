mnt produce 0.1.0
-----------------

a new, minimal linux DAW (mainly a sequencer) for hackers.

- scalable vector UI based on the GLV toolkit
- minilisp as scripting language / project file format and for keybindings
- connects via JACK MIDI to synths and audio hardware
- connects via multiple JACK Audio outputs to plugin racks for mixing

![produce screenshot from feb 22, 2015](http://dump.mntmn.com/produce-shot-feb22.png)

design goals
------------

- do one thing well: comfortable editing of tracks and regions.
- focus on speed, responsiveness and productivity.
- no "modes" or "tools".
- integrate well: produce is designed to work well in a JACK environment and leverage its robust mixing and routing.
- be visually pleasing: reduce clutter and balance colors. work well on high resolution screens.

mouse commands
--------------

- click on a track: select that track, deselect all regions
- click on a region: select that region, deselect all others
- shift-click on a region: add that to selection
- drag on track: selection rectangle, selects multiple regions at once
- drag regions: change region start points
- ctrl-drag regions: drag a copy of the selected regions to start points at mouse cursor
- drop audio file (e.g. from thunar): create new track whose regions will trigger a converted version of the dropped audio file

dragged and added regions will snap to 1/2 beat points.

default keyboard commands
-------------------------

- ````-```` zoom out
- ````+```` zoom in
- ````left```` scroll left
- ````right```` scroll right
- ````,```` move playhead left
- ````.```` move playhead right
- ````space```` toggle play/pause
- ````l```` load project
- ````s```` save project (will overwrite the current project file!) 
- ````d```` delete selected track
- ````t```` create empty track (currently useless because you cannot change a track configuration yet)
- ````a```` create region in mouse-hovered track at hovered position
- ````1```` set duration of selected regions to 1/4 beat
- ````2```` set duration of selected regions to 1/2 beat
- ````3```` set duration of selected regions to 1 beat
- ````4```` set duration of selected regions to 2 beats
- ````b```` "bounce" project (from loop in to loop out marker); this will play back your song from loop in to loop out and record the JACK system output to a file called jack_capure_XXX.wav where XXX is an increasing number. 

keyboard bindings can be adjusted by editing ````init.l````.

other GUI features
------------------

- loop in (white square) and loop out (pink square) markers can be dragged to define loop/project area
- edit or drag BPM (beats per minute) number in upper left corner

bugs/missing features
---------------------

- grid supports only 4/4 measure
- GLV has messed up keyboard scan codes, so project name text field will behave strangely; for me, del/backspace are swapped, cursor keys produce characters; will replace with zenity
- tested on 2560x1600 resolution; element size factors hardcoded, will be moved to init.l
- velocity not editable yet
- fixed maximum project size 400 bars; will be editable soon

dependencies
------------

- ````sox```` for audio file conversion (optional, required if you drag in audio files)
- ````zenity```` for text input dialogs (optional, required for lisp evaluation)
- ````mhwaveedit```` for editing audio files (optional, required if you want to edit audio files from within a produce project) 
- modified GLV toolkit (included, modified font drawing line width)
- modified freeGLUT (included, modified to allow responding to X11 events like drag&drop)
- libsndfile
- X11

highly recommended
------------------

- LADI session handler / ````gladish```` is useful for saving and loading your session state, JACK connections and starting all the necessary applications.
- ````bristol```` emulates a ton of vintage synths and can be controlled by produce via MIDI ports
- ````calfjackhost```` for running an effects plugin stack.

building / running
------------------

the included ````produce```` binary and libs are built on x86_64 debian 8. no other distributions have been tested, but your feedback is appreciated.

if you want to build yourself, do this:

1. install build dependencies: ````apt-get install libsndfile-dev libgl1-mesa-dev libglu1-mesa-dev libglew-dev libx11-dev libxt-dev libxi-dev libxrandr-dev libjack-jackd2-dev g++ build-essential````
2. build modified libraries: ````./build_deps.sh````
3. build produce: ````./build.sh````
4. run: ````./produce.sh````

quickstart
----------

1. drop in some audio samples (one at a time). a track for each will be created
2. press ````a```` while hovering over empty positions in the track; regions will be created
3. select all regions by clicking or dragging up the selection rectangle
4. press ````1```` to set all regions to 1/4 duration
5. *important* by default, you will hear nothing. you need to connect produce's JACK audio outputs to your system outputs. the easiest way is to do this in ````qjackctl```` or ````gladish```` (preferred). in the latter, just drag lines from produce's audio output ports to your system output ports.
6. press ````space```` to toggle playback. you should hear something.
7. to add a midi octave starting with note C3 on midi port 0, press ````(````. a popup should appear. enter ````(octave 3 0)````. 12 tracks will be created, labeled with their MIDI note numbers.
8. launch your favorite synth app (or connect a hardware synth), for example ````monobristol````, set the MIDI channel to 1 (or omni).
9. using ````gladish````, connect produce's MIDI output port 0 to your synth.
10. create MIDI regions by hovering over your MIDI tracks and press ````a```` to create actual notes.
11. toggle playback using ````space```` and your synth should play back your notes.
12. press ````s```` to save your project. next time, load it using ````l````.

license / credits
-----------------

produce is free software.

license: MIT

copyright 2014-2015 Lukas F. Hartmann / @mntmn / http://mnt.mn
