# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.0

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mntmn/code/produce/freeglut

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mntmn/code/produce/freeglut

# Include any dependencies generated for this target.
include CMakeFiles/smooth_opengl3_static.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/smooth_opengl3_static.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/smooth_opengl3_static.dir/flags.make

CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o: CMakeFiles/smooth_opengl3_static.dir/flags.make
CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o: progs/demos/smooth_opengl3/smooth_opengl3.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/mntmn/code/produce/freeglut/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o   -c /home/mntmn/code/produce/freeglut/progs/demos/smooth_opengl3/smooth_opengl3.c

CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/mntmn/code/produce/freeglut/progs/demos/smooth_opengl3/smooth_opengl3.c > CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.i

CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/mntmn/code/produce/freeglut/progs/demos/smooth_opengl3/smooth_opengl3.c -o CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.s

CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o.requires:
.PHONY : CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o.requires

CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o.provides: CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o.requires
	$(MAKE) -f CMakeFiles/smooth_opengl3_static.dir/build.make CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o.provides.build
.PHONY : CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o.provides

CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o.provides.build: CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o

# Object files for target smooth_opengl3_static
smooth_opengl3_static_OBJECTS = \
"CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o"

# External object files for target smooth_opengl3_static
smooth_opengl3_static_EXTERNAL_OBJECTS =

bin/smooth_opengl3_static: CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o
bin/smooth_opengl3_static: CMakeFiles/smooth_opengl3_static.dir/build.make
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libGLU.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libGL.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libSM.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libICE.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libX11.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libXext.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libXrandr.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libXi.so
bin/smooth_opengl3_static: lib/libglut.a
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libGL.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libSM.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libICE.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libX11.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libXext.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libXrandr.so
bin/smooth_opengl3_static: /usr/lib/x86_64-linux-gnu/libXi.so
bin/smooth_opengl3_static: CMakeFiles/smooth_opengl3_static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable bin/smooth_opengl3_static"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/smooth_opengl3_static.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/smooth_opengl3_static.dir/build: bin/smooth_opengl3_static
.PHONY : CMakeFiles/smooth_opengl3_static.dir/build

CMakeFiles/smooth_opengl3_static.dir/requires: CMakeFiles/smooth_opengl3_static.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.o.requires
.PHONY : CMakeFiles/smooth_opengl3_static.dir/requires

CMakeFiles/smooth_opengl3_static.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/smooth_opengl3_static.dir/cmake_clean.cmake
.PHONY : CMakeFiles/smooth_opengl3_static.dir/clean

CMakeFiles/smooth_opengl3_static.dir/depend:
	cd /home/mntmn/code/produce/freeglut && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mntmn/code/produce/freeglut /home/mntmn/code/produce/freeglut /home/mntmn/code/produce/freeglut /home/mntmn/code/produce/freeglut /home/mntmn/code/produce/freeglut/CMakeFiles/smooth_opengl3_static.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/smooth_opengl3_static.dir/depend

