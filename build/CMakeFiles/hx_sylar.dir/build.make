# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/hx/hx_sylar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hx/hx_sylar/build

# Include any dependencies generated for this target.
include CMakeFiles/hx_sylar.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/hx_sylar.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/hx_sylar.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/hx_sylar.dir/flags.make

CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.o: CMakeFiles/hx_sylar.dir/flags.make
CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.o: ../hx_sylar/config.cc
CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.o: CMakeFiles/hx_sylar.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.o"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/config.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.o -MF CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.o.d -o CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.o -c /home/hx/hx_sylar/hx_sylar/config.cc

CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.i"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/config.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hx/hx_sylar/hx_sylar/config.cc > CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.i

CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.s"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/config.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hx/hx_sylar/hx_sylar/config.cc -o CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.s

CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.o: CMakeFiles/hx_sylar.dir/flags.make
CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.o: ../hx_sylar/log.cc
CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.o: CMakeFiles/hx_sylar.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.o"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/log.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.o -MF CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.o.d -o CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.o -c /home/hx/hx_sylar/hx_sylar/log.cc

CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.i"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/log.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hx/hx_sylar/hx_sylar/log.cc > CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.i

CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.s"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/log.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hx/hx_sylar/hx_sylar/log.cc -o CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.s

CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.o: CMakeFiles/hx_sylar.dir/flags.make
CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.o: ../hx_sylar/thread.cc
CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.o: CMakeFiles/hx_sylar.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.o"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.o -MF CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.o.d -o CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.o -c /home/hx/hx_sylar/hx_sylar/thread.cc

CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.i"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hx/hx_sylar/hx_sylar/thread.cc > CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.i

CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.s"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hx/hx_sylar/hx_sylar/thread.cc -o CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.s

CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.o: CMakeFiles/hx_sylar.dir/flags.make
CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.o: ../hx_sylar/iomanager.cc
CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.o: CMakeFiles/hx_sylar.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.o"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/iomanager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.o -MF CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.o.d -o CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.o -c /home/hx/hx_sylar/hx_sylar/iomanager.cc

CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.i"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/iomanager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hx/hx_sylar/hx_sylar/iomanager.cc > CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.i

CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.s"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/iomanager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hx/hx_sylar/hx_sylar/iomanager.cc -o CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.s

CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.o: CMakeFiles/hx_sylar.dir/flags.make
CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.o: ../hx_sylar/util.cc
CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.o: CMakeFiles/hx_sylar.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.o"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/util.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.o -MF CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.o.d -o CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.o -c /home/hx/hx_sylar/hx_sylar/util.cc

CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.i"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/util.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hx/hx_sylar/hx_sylar/util.cc > CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.i

CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.s"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/util.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hx/hx_sylar/hx_sylar/util.cc -o CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.s

CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.o: CMakeFiles/hx_sylar.dir/flags.make
CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.o: ../hx_sylar/fiber.cc
CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.o: CMakeFiles/hx_sylar.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.o"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/fiber.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.o -MF CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.o.d -o CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.o -c /home/hx/hx_sylar/hx_sylar/fiber.cc

CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.i"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/fiber.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hx/hx_sylar/hx_sylar/fiber.cc > CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.i

CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.s"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/fiber.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hx/hx_sylar/hx_sylar/fiber.cc -o CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.s

CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.o: CMakeFiles/hx_sylar.dir/flags.make
CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.o: ../hx_sylar/scheduler.cc
CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.o: CMakeFiles/hx_sylar.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.o"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/scheduler.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.o -MF CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.o.d -o CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.o -c /home/hx/hx_sylar/hx_sylar/scheduler.cc

CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.i"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/scheduler.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hx/hx_sylar/hx_sylar/scheduler.cc > CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.i

CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.s"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"hx_sylar/scheduler.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hx/hx_sylar/hx_sylar/scheduler.cc -o CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.s

# Object files for target hx_sylar
hx_sylar_OBJECTS = \
"CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.o" \
"CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.o" \
"CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.o" \
"CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.o" \
"CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.o" \
"CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.o" \
"CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.o"

# External object files for target hx_sylar
hx_sylar_EXTERNAL_OBJECTS =

libhx_sylar.so: CMakeFiles/hx_sylar.dir/hx_sylar/config.cc.o
libhx_sylar.so: CMakeFiles/hx_sylar.dir/hx_sylar/log.cc.o
libhx_sylar.so: CMakeFiles/hx_sylar.dir/hx_sylar/thread.cc.o
libhx_sylar.so: CMakeFiles/hx_sylar.dir/hx_sylar/iomanager.cc.o
libhx_sylar.so: CMakeFiles/hx_sylar.dir/hx_sylar/util.cc.o
libhx_sylar.so: CMakeFiles/hx_sylar.dir/hx_sylar/fiber.cc.o
libhx_sylar.so: CMakeFiles/hx_sylar.dir/hx_sylar/scheduler.cc.o
libhx_sylar.so: CMakeFiles/hx_sylar.dir/build.make
libhx_sylar.so: CMakeFiles/hx_sylar.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX shared library libhx_sylar.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hx_sylar.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/hx_sylar.dir/build: libhx_sylar.so
.PHONY : CMakeFiles/hx_sylar.dir/build

CMakeFiles/hx_sylar.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/hx_sylar.dir/cmake_clean.cmake
.PHONY : CMakeFiles/hx_sylar.dir/clean

CMakeFiles/hx_sylar.dir/depend:
	cd /home/hx/hx_sylar/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hx/hx_sylar /home/hx/hx_sylar /home/hx/hx_sylar/build /home/hx/hx_sylar/build /home/hx/hx_sylar/build/CMakeFiles/hx_sylar.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/hx_sylar.dir/depend

