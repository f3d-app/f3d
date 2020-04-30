title: Installation and build
---

# Installation

You can find the release binary packages for Windows, Linux, and OSX on the [Release page](https://gitlab.kitware.com/f3d/f3d/-/releases).
Alternatively, you can build it yourself, following the build guide.

# Build

## Dependencies
* [CMake](https://cmake.org)
* [VTK](https://vtk.org) (optionally with raytracing capabilities to enable OSPray rendering).
* A C++11 compiler
* A CMake-compatible build system (Visual Studio, XCode, Ninja, Make...)

## Configuration and building
Set the following CMake options:
* `VTK_DIR`: Point to a build or install directory of VTK.
* `BUILD_TESTING`: Optionally, enable the tests.
* `MACOSX_BUILD_BUNDLE`: On macOS, build a `.app` bundle.
* `WINDOWS_BUILD_WIN32`: On Windows, build a Win32 application (without console).

Then build the software using your build system.
