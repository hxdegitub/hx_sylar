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
include CMakeFiles/test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test.dir/flags.make

CMakeFiles/test.dir/tests/test.cc.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/tests/test.cc.o: ../tests/test.cc
CMakeFiles/test.dir/tests/test.cc.o: CMakeFiles/test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test.dir/tests/test.cc.o"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"tests/test.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test.dir/tests/test.cc.o -MF CMakeFiles/test.dir/tests/test.cc.o.d -o CMakeFiles/test.dir/tests/test.cc.o -c /home/hx/hx_sylar/tests/test.cc

CMakeFiles/test.dir/tests/test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/tests/test.cc.i"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"tests/test.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hx/hx_sylar/tests/test.cc > CMakeFiles/test.dir/tests/test.cc.i

CMakeFiles/test.dir/tests/test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/tests/test.cc.s"
	/usr/bin/g++ $(CXX_DEFINES) -D__FILE__=\"tests/test.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hx/hx_sylar/tests/test.cc -o CMakeFiles/test.dir/tests/test.cc.s

# Object files for target test
test_OBJECTS = \
"CMakeFiles/test.dir/tests/test.cc.o"

# External object files for target test
test_EXTERNAL_OBJECTS =

../bin/test: CMakeFiles/test.dir/tests/test.cc.o
../bin/test: CMakeFiles/test.dir/build.make
../bin/test: libhx_sylar.so
../bin/test: /usr/lib/x86_64-linux-gnu/libyaml-cpp.so
../bin/test: CMakeFiles/test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test.dir/build: ../bin/test
.PHONY : CMakeFiles/test.dir/build

CMakeFiles/test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test.dir/clean

CMakeFiles/test.dir/depend:
	cd /home/hx/hx_sylar/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hx/hx_sylar /home/hx/hx_sylar /home/hx/hx_sylar/build /home/hx/hx_sylar/build /home/hx/hx_sylar/build/CMakeFiles/test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test.dir/depend
