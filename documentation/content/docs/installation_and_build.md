title: Installation and build
---

# How to use

There are 4 main ways to use F3D:

* By running F3D from a terminal with a set of command-line options.
* By running F3D directly and then dragging and dropping files into it to open them.
* By using F3D automatically in the file manager when opening file.
* As a thumbnailer for all supported file formats with certain file managers.

# Installation

You can find the release binary packages for Windows, Linux and macOS on the [Release page](https://github.com/f3d-app/f3d/releases).
Alternatively, you can build it yourself following the [build](#Build) guide below.

You can also find packages for the following operating systems:

## Ubuntu/Debian/openSuse

Available on [OpenSuse OBS](https://build.opensuse.org/package/show/home:AndnoVember:F3D/f3d).

## Arch Linux

Available in the [AUR](https://aur.archlinux.org/packages/f3d).

## FreeBSD

Avaiable in [FreshPORTS](https://www.freshports.org/graphics/f3d).

## DragonFly BSD

Available in [DPorts](https://github.com/DragonFlyBSD/DPorts/tree/master/graphics/f3d).

## MacOS Brew

Available in [Homebrew](https://formulae.brew.sh/formula/f3d).

## NixOS

Available in [nixpkgs](https://github.com/NixOS/nixpkgs/blob/release-21.05/pkgs/applications/graphics/f3d/default.nix).

## Flathub

Available in [Flathub](https://flathub.org/apps/details/io.github.f3d_app.f3d).

# Build

## Dependencies

* [CMake](https://cmake.org) >= 3.1.
* [VTK](https://vtk.org) >= 9.0.0 (optionally with raytracing capabilities to enable OSPray rendering).
* A C++11 compiler.
* A CMake-compatible build system (Visual Studio, XCode, Ninja, Make, etc.).

## Configuration and building

Configure and generate the project with CMake by providing the following CMake options:

* `VTK_DIR`: Path to a build or install directory of VTK.
* `BUILD_TESTING`: Enable the tests.
* `F3D_MACOS_BUNDLE`: On macOS, build a `.app` bundle.
* `F3D_WINDOWS_GUI`: On Windows, build a Win32 application (without console).

Some modules depending on external libraries can be optionally enabled with the following CMake variables:

* `F3D_MODULE_EXODUS`: Support for ExodusII (.ex2) file format. Requires that VTK has been built with `IOExodus` module (and `hdf5`). Enabled by default.
* `F3D_MODULE_RAYTRACING`: Support for raytracing rendering. Requires that VTK has been built with `OSPRay`. Disabled by default.
* `F3D_MODULE_OCCT`: Support for STEP and IGES file formats. Requires `OpenCASCADE`. Disabled by default.
* `F3D_MODULE_ASSIMP`: Support for FBX, DAE, OFF and DXF file formats. Requires `Assimp`. Disabled by default.

Then build the software using your build system.
