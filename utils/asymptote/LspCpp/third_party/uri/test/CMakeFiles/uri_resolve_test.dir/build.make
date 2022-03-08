# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

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
CMAKE_SOURCE_DIR = /u/bowman/camp/LspCpp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /u/bowman/camp/LspCpp

# Include any dependencies generated for this target.
include third_party/uri/test/CMakeFiles/uri_resolve_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include third_party/uri/test/CMakeFiles/uri_resolve_test.dir/compiler_depend.make

# Include the progress variables for this target.
include third_party/uri/test/CMakeFiles/uri_resolve_test.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/uri/test/CMakeFiles/uri_resolve_test.dir/flags.make

third_party/uri/test/CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.o: third_party/uri/test/CMakeFiles/uri_resolve_test.dir/flags.make
third_party/uri/test/CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.o: third_party/uri/test/uri_resolve_test.cpp
third_party/uri/test/CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.o: third_party/uri/test/CMakeFiles/uri_resolve_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/u/bowman/camp/LspCpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object third_party/uri/test/CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.o"
	cd /u/bowman/camp/LspCpp/third_party/uri/test && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT third_party/uri/test/CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.o -MF CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.o.d -o CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.o -c /u/bowman/camp/LspCpp/third_party/uri/test/uri_resolve_test.cpp

third_party/uri/test/CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.i"
	cd /u/bowman/camp/LspCpp/third_party/uri/test && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /u/bowman/camp/LspCpp/third_party/uri/test/uri_resolve_test.cpp > CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.i

third_party/uri/test/CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.s"
	cd /u/bowman/camp/LspCpp/third_party/uri/test && /bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /u/bowman/camp/LspCpp/third_party/uri/test/uri_resolve_test.cpp -o CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.s

# Object files for target uri_resolve_test
uri_resolve_test_OBJECTS = \
"CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.o"

# External object files for target uri_resolve_test
uri_resolve_test_EXTERNAL_OBJECTS =

third_party/uri/tests/uri_resolve_test: third_party/uri/test/CMakeFiles/uri_resolve_test.dir/uri_resolve_test.cpp.o
third_party/uri/tests/uri_resolve_test: third_party/uri/test/CMakeFiles/uri_resolve_test.dir/build.make
third_party/uri/tests/uri_resolve_test: third_party/uri/src/libnetwork-uri.a
third_party/uri/tests/uri_resolve_test: lib/libgtest_main.a
third_party/uri/tests/uri_resolve_test: lib/libgtest.a
third_party/uri/tests/uri_resolve_test: third_party/uri/test/CMakeFiles/uri_resolve_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/u/bowman/camp/LspCpp/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../tests/uri_resolve_test"
	cd /u/bowman/camp/LspCpp/third_party/uri/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/uri_resolve_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
third_party/uri/test/CMakeFiles/uri_resolve_test.dir/build: third_party/uri/tests/uri_resolve_test
.PHONY : third_party/uri/test/CMakeFiles/uri_resolve_test.dir/build

third_party/uri/test/CMakeFiles/uri_resolve_test.dir/clean:
	cd /u/bowman/camp/LspCpp/third_party/uri/test && $(CMAKE_COMMAND) -P CMakeFiles/uri_resolve_test.dir/cmake_clean.cmake
.PHONY : third_party/uri/test/CMakeFiles/uri_resolve_test.dir/clean

third_party/uri/test/CMakeFiles/uri_resolve_test.dir/depend:
	cd /u/bowman/camp/LspCpp && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /u/bowman/camp/LspCpp /u/bowman/camp/LspCpp/third_party/uri/test /u/bowman/camp/LspCpp /u/bowman/camp/LspCpp/third_party/uri/test /u/bowman/camp/LspCpp/third_party/uri/test/CMakeFiles/uri_resolve_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : third_party/uri/test/CMakeFiles/uri_resolve_test.dir/depend

