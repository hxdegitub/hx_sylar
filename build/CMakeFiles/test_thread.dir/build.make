# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/hx/hx_sylar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/hx/hx_sylar/build

# Include any dependencies generated for this target.
include CMakeFiles/test_thread.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test_thread.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test_thread.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_thread.dir/flags.make

CMakeFiles/test_thread.dir/tests/test_thread.cc.o: CMakeFiles/test_thread.dir/flags.make
CMakeFiles/test_thread.dir/tests/test_thread.cc.o: /home/hx/hx_sylar/tests/test_thread.cc
CMakeFiles/test_thread.dir/tests/test_thread.cc.o: CMakeFiles/test_thread.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_thread.dir/tests/test_thread.cc.o"
	/usr/bin/clang++ $(CXX_DEFINES) -D__FILE__=\"tests/test_thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_thread.dir/tests/test_thread.cc.o -MF CMakeFiles/test_thread.dir/tests/test_thread.cc.o.d -o CMakeFiles/test_thread.dir/tests/test_thread.cc.o -c /home/hx/hx_sylar/tests/test_thread.cc

CMakeFiles/test_thread.dir/tests/test_thread.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test_thread.dir/tests/test_thread.cc.i"
	/usr/bin/clang++ $(CXX_DEFINES) -D__FILE__=\"tests/test_thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/hx/hx_sylar/tests/test_thread.cc > CMakeFiles/test_thread.dir/tests/test_thread.cc.i

CMakeFiles/test_thread.dir/tests/test_thread.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test_thread.dir/tests/test_thread.cc.s"
	/usr/bin/clang++ $(CXX_DEFINES) -D__FILE__=\"tests/test_thread.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/hx/hx_sylar/tests/test_thread.cc -o CMakeFiles/test_thread.dir/tests/test_thread.cc.s

# Object files for target test_thread
test_thread_OBJECTS = \
"CMakeFiles/test_thread.dir/tests/test_thread.cc.o"

# External object files for target test_thread
test_thread_EXTERNAL_OBJECTS =

/home/hx/hx_sylar/bin/test_thread: CMakeFiles/test_thread.dir/tests/test_thread.cc.o
/home/hx/hx_sylar/bin/test_thread: CMakeFiles/test_thread.dir/build.make
/home/hx/hx_sylar/bin/test_thread: libhx_sylar.so
/home/hx/hx_sylar/bin/test_thread: /usr/lib/x86_64-linux-gnu/libyaml-cpp.so
/home/hx/hx_sylar/bin/test_thread: CMakeFiles/test_thread.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/hx/hx_sylar/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/hx/hx_sylar/bin/test_thread"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_thread.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_thread.dir/build: /home/hx/hx_sylar/bin/test_thread
.PHONY : CMakeFiles/test_thread.dir/build

CMakeFiles/test_thread.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_thread.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_thread.dir/clean

CMakeFiles/test_thread.dir/depend:
	cd /home/hx/hx_sylar/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/hx/hx_sylar /home/hx/hx_sylar /home/hx/hx_sylar/build /home/hx/hx_sylar/build /home/hx/hx_sylar/build/CMakeFiles/test_thread.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/test_thread.dir/depend

