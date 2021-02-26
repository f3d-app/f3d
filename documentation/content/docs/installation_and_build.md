title: Installation and build
---

# Installation

You can find the release binary packages for Windows, Linux, and macOS on the [Release page](https://gitlab.kitware.com/f3d/f3d/-/releases).
Alternatively, you can build it yourself, following the build guide.

# How to use

There is mainly three ways to use F3D.

 * By running F3D from a terminal with command-line options.
 * By running F3D directly and then using drag&drop files into it to open them.
 * By using F3D as an "Open with" program with specific file types.

# Build

## Dependencies
* [CMake](https://cmake.org) >= 3.1 .
* [VTK](https://vtk.org) >= 9.0.0 (optionally with raytracing capabilities to enable OSPray rendering).
* A C++11 compiler.
* A CMake-compatible build system (Visual Studio, XCode, Ninja, Make...).

## Configuration and building
Set the following CMake options:
* `VTK_DIR`: Point to a build or install directory of VTK.
* `BUILD_TESTING`: Optionally, enable the tests.
* `F3D_MACOS_BUNDLE`: On macOS, build a `.app` bundle.
* `F3D_WINDOWS_GUI`: On Windows, build a Win32 application (without console).

Then build the software using your build system.
