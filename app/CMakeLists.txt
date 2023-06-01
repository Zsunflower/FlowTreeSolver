# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set( OpenCV_DIR E:/OpenCV-android-sdk/sdk/native/jni)

find_package(OpenCV REQUIRED )
if(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
    message(STATUS "OpenCV library status:")
    message(STATUS "    version: ${OpenCV_VERSION}")
    message(STATUS "    libraries: ${OpenCV_LIBS}")
    message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
else(OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV library not found")
endif(OpenCV_FOUND)

#include_directories(E:/AndroidProjects/FlowTreeSolver/app/src/main/cpp/minisat)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
set(minisat_SOURCE_DIR E:/AndroidProjects/FlowTreeSolver/app/src/main/cpp)
include_directories(${minisat_SOURCE_DIR})

add_library( # Sets the name of the library.
             Solver

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/board.h
             src/main/cpp/board.cpp
             src/main/cpp/preprocessing.h
             src/main/cpp/preprocessing.cpp
             src/main/cpp/FTSolver.h
             src/main/cpp/FTSolver.cpp
             src/main/cpp/FlowTreeJNI.h
             src/main/cpp/FlowTreeJNI.cpp
             src/main/cpp/minisat/utils/Options.cc
             src/main/cpp/minisat/utils/System.cc
             src/main/cpp/minisat/core/Solver.cc
             src/main/cpp/minisat/simp/SimpSolver.cc )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       Solver
                       ${OpenCV_LIBS}
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )