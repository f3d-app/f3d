---
sidebar_position: 1
---

# Getting started guide

This is a getting started guide, aimed to people that are not used
to C++ programming, configuring and compiling. If you are already used
to such processes, you may be looking for our [build guide](BUILD.md).

This guide will help you to compile your own version of F3D with required dependencies.
Optional dependencies or covered in the build guide.

 - If you are using [Linux](#Linux)
 - If you are using [Windows](#Windows)
 - If you are using [macOS](#macOS)

## Linux

Note: The following steps were tested with Ubuntu 22 and ArchLinux but
should work for other OSes as stated, as long as listed packages are available.

### Install dependencies

First make sure your system is up-to-date,
then install the required dependencies using the terminal.

#### Debian/Ubuntu/Mint

```
sudo apt install build-essential git git-lfs cmake libvtk9-dev
```

#### Fedora/Centos/RedHat

```
sudo yum install make automake gcc gcc-c++ kernel-devel git git-lfs cmake vtk
```

#### Arch Linux

```
sudo pacman -S base-devel git git-lfs cmake vtk
```

### Compile F3D

Open a terminal.

```
mkdir f3d
cd f3d
git clone https://github.com/f3d-app/f3d src
mkdir build
cd build
cmake ../src
make
```

You can then simply run F3D from the command line:

```
./bin/f3d
```

## Windows

Note: The following steps were tested with Visual Studio Community 2022 using Windows 10, but should also work for other versions

#### Install Dependencies
 * Download and install [git bash for windows][gitforwindows]
 * Download and install [cmake][cmake-download]
 * Download and install [Visual Studio 2022 Community Edition][visual-studio]

#### Recover F3D sources

 * Open git bash

```sh
cd C:
mkdir dev
cd dev
mkdir f3d
cd f3d
git clone https://github.com/f3d-app/f3d.git src
mkdir build
```

#### Install dependencies using vcpkg

 * Open git bash

```sh
cd C:/dev
git clone https://github.com/Microsoft/vcpkg.git
```

 * Open cmd

```sh
cd C:\dev
./vcpkg/bootstrap-vcpkg.sh
cmake -B ./f3d/build -S ./f3d/src -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake 
```

Note: Last command will take a while. It download, compile and install all dependencies, including
many optional ones.

#### Build F3D

 * Open cmd

```sh
cd C:\dev\f3d\build
cmake .
cmake --build .
```

#### Run

 * Double click on the `C:\dev\f3d\build\bin_Debug\f3d.exe` executable

## macOS

Note: The following steps were tested with macOS Catalina using Xcode 12.4, but should also work for other versions

### Install Xcode

There are different ways to install Xcode, you can use the AppStore or download it
from apple developer website and then extract and install it.

### Install Homebrew

Open a terminal.

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
(echo; echo 'eval "$(/opt/homebrew/bin/brew shellenv)"') >> ~/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"
```

### Install dependencies

Open a terminal.

```
brew install git git-lfs cmake curl vtk
git lfs install
```

Note: Homebrew is downloading, compiling and installing all vtk dependencies.
This command may take a very long time.

### Compile F3D

Open a terminal.

```
mkdir f3d
cd f3d
git clone https://github.com/f3d-app/f3d src
mkdir build
cd build
cmake ../src
make
```

You can then simply run F3D from the command line:

```
./bin/f3d.app/Contents/MacOS/f3d
```

[cmake-download]: https://cmake.org/download
[gitforwindows]: https://gitforwindows.org/
[visual-studio]: https://visualstudio.microsoft.com/vs/community/
