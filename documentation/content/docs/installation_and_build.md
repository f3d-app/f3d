title: Installation and build
---

# Installation

You can find the release binary packages for Windows, Linux, and macOS on the [Release page](https://gitlab.kitware.com/f3d/f3d/-/releases).
Alternatively, you can build it yourself, following the build guide below.

# How to use

There are 4 main ways to use F3D:

* By running F3D from a terminal with a set of command-line options.
* By running F3D directly and then dragging and dropping files into it to open them.
* By using F3D as an "Open with" program with specific file types.
* As a thumbnailer for all supported file formats with certain file managers.

# Build

## Dependencies

* [CMake](https://cmake.org) >= 3.1 .
* [VTK](https://vtk.org) >= 9.0.0 (optionally with raytracing capabilities to enable OSPray rendering).
* A C++11 compiler.
* A CMake-compatible build system (Visual Studio, XCode, Ninja, Make, etc.).

## Configuration and building

Configure and generate the project with CMake by providing the following CMake options:

* `VTK_DIR`: Path to a build or install directory of VTK.
* `BUILD_TESTING`: Enable the tests.
* `F3D_MACOS_BUNDLE`: On macOS, build a `.app` bundle.
* `WINDOWS_F3D_WINDOWS_GUIBUILD_WIN32`: On Windows, build a Win32 application (without console).

Some modules depending on external libraries can be optionally enabled with the following CMake variables:

* `F3D_MODULE_EXODUS`: Support for ExodusII (.ex2) file format. Requires that VTK has been built with `IOExodus` module (and `hdf5`). Enabled by default.
* `F3D_MODULE_RAYTRACING`: Support for raytracing rendering. Requires that VTK has been built with `OSPRay`. Disabled by default.
* `F3D_MODULE_OCCT`: Support for STEP and IGES file formats. Requires `OpenCASCADE`. Disabled by default.

Then build the software using your build system.
