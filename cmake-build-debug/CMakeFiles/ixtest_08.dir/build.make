# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/jiayi/Desktop/CMPS181-Project-3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/ixtest_08.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ixtest_08.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ixtest_08.dir/flags.make

CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o: CMakeFiles/ixtest_08.dir/flags.make
CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o: ../rbf/pfm.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o -c /Users/jiayi/Desktop/CMPS181-Project-3/rbf/pfm.cc

CMakeFiles/ixtest_08.dir/rbf/pfm.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ixtest_08.dir/rbf/pfm.cc.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jiayi/Desktop/CMPS181-Project-3/rbf/pfm.cc > CMakeFiles/ixtest_08.dir/rbf/pfm.cc.i

CMakeFiles/ixtest_08.dir/rbf/pfm.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ixtest_08.dir/rbf/pfm.cc.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jiayi/Desktop/CMPS181-Project-3/rbf/pfm.cc -o CMakeFiles/ixtest_08.dir/rbf/pfm.cc.s

CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o.requires:

.PHONY : CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o.requires

CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o.provides: CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o.requires
	$(MAKE) -f CMakeFiles/ixtest_08.dir/build.make CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o.provides.build
.PHONY : CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o.provides

CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o.provides.build: CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o


CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o: CMakeFiles/ixtest_08.dir/flags.make
CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o: ../rbf/rbfm.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o -c /Users/jiayi/Desktop/CMPS181-Project-3/rbf/rbfm.cc

CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jiayi/Desktop/CMPS181-Project-3/rbf/rbfm.cc > CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.i

CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jiayi/Desktop/CMPS181-Project-3/rbf/rbfm.cc -o CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.s

CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o.requires:

.PHONY : CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o.requires

CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o.provides: CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o.requires
	$(MAKE) -f CMakeFiles/ixtest_08.dir/build.make CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o.provides.build
.PHONY : CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o.provides

CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o.provides.build: CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o


CMakeFiles/ixtest_08.dir/ix/ix.cc.o: CMakeFiles/ixtest_08.dir/flags.make
CMakeFiles/ixtest_08.dir/ix/ix.cc.o: ../ix/ix.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/ixtest_08.dir/ix/ix.cc.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ixtest_08.dir/ix/ix.cc.o -c /Users/jiayi/Desktop/CMPS181-Project-3/ix/ix.cc

CMakeFiles/ixtest_08.dir/ix/ix.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ixtest_08.dir/ix/ix.cc.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jiayi/Desktop/CMPS181-Project-3/ix/ix.cc > CMakeFiles/ixtest_08.dir/ix/ix.cc.i

CMakeFiles/ixtest_08.dir/ix/ix.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ixtest_08.dir/ix/ix.cc.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jiayi/Desktop/CMPS181-Project-3/ix/ix.cc -o CMakeFiles/ixtest_08.dir/ix/ix.cc.s

CMakeFiles/ixtest_08.dir/ix/ix.cc.o.requires:

.PHONY : CMakeFiles/ixtest_08.dir/ix/ix.cc.o.requires

CMakeFiles/ixtest_08.dir/ix/ix.cc.o.provides: CMakeFiles/ixtest_08.dir/ix/ix.cc.o.requires
	$(MAKE) -f CMakeFiles/ixtest_08.dir/build.make CMakeFiles/ixtest_08.dir/ix/ix.cc.o.provides.build
.PHONY : CMakeFiles/ixtest_08.dir/ix/ix.cc.o.provides

CMakeFiles/ixtest_08.dir/ix/ix.cc.o.provides.build: CMakeFiles/ixtest_08.dir/ix/ix.cc.o


CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o: CMakeFiles/ixtest_08.dir/flags.make
CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o: ../ix/ixtest_08.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o -c /Users/jiayi/Desktop/CMPS181-Project-3/ix/ixtest_08.cc

CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jiayi/Desktop/CMPS181-Project-3/ix/ixtest_08.cc > CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.i

CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jiayi/Desktop/CMPS181-Project-3/ix/ixtest_08.cc -o CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.s

CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o.requires:

.PHONY : CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o.requires

CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o.provides: CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o.requires
	$(MAKE) -f CMakeFiles/ixtest_08.dir/build.make CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o.provides.build
.PHONY : CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o.provides

CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o.provides.build: CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o


# Object files for target ixtest_08
ixtest_08_OBJECTS = \
"CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o" \
"CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o" \
"CMakeFiles/ixtest_08.dir/ix/ix.cc.o" \
"CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o"

# External object files for target ixtest_08
ixtest_08_EXTERNAL_OBJECTS =

ixtest_08: CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o
ixtest_08: CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o
ixtest_08: CMakeFiles/ixtest_08.dir/ix/ix.cc.o
ixtest_08: CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o
ixtest_08: CMakeFiles/ixtest_08.dir/build.make
ixtest_08: CMakeFiles/ixtest_08.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable ixtest_08"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ixtest_08.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ixtest_08.dir/build: ixtest_08

.PHONY : CMakeFiles/ixtest_08.dir/build

CMakeFiles/ixtest_08.dir/requires: CMakeFiles/ixtest_08.dir/rbf/pfm.cc.o.requires
CMakeFiles/ixtest_08.dir/requires: CMakeFiles/ixtest_08.dir/rbf/rbfm.cc.o.requires
CMakeFiles/ixtest_08.dir/requires: CMakeFiles/ixtest_08.dir/ix/ix.cc.o.requires
CMakeFiles/ixtest_08.dir/requires: CMakeFiles/ixtest_08.dir/ix/ixtest_08.cc.o.requires

.PHONY : CMakeFiles/ixtest_08.dir/requires

CMakeFiles/ixtest_08.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ixtest_08.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ixtest_08.dir/clean

CMakeFiles/ixtest_08.dir/depend:
	cd /Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/jiayi/Desktop/CMPS181-Project-3 /Users/jiayi/Desktop/CMPS181-Project-3 /Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug /Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug /Users/jiayi/Desktop/CMPS181-Project-3/cmake-build-debug/CMakeFiles/ixtest_08.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ixtest_08.dir/depend

