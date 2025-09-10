# Getting started guide

This is a getting started guide, aimed to people that are not used
to C++ programming, configuring and compiling. If you are already used
to such processes, you may be looking for our [build guide](BUILD.md).

You may also want to take a look into the [mentoring program](MENTORING.md).

This guide will help you to compile your own version of F3D with required dependencies.
Optional dependencies or covered in the build guide.

- If you are using [Linux](#linux)
- If you are using [Windows](#windows)
- If you are using [macOS](#macos)

## Linux

Note: The following steps were tested with Ubuntu 23 and ArchLinux but
should work for other OSes as stated, as long as listed packages are available.

### Install dependencies

First make sure your system is up-to-date,
then install the required dependencies using the terminal.

#### Debian/Ubuntu/Mint

```
sudo apt update
sudo apt upgrade
sudo apt install build-essential git git-lfs cmake libvtk9-dev
```

Note: Ubuntu 24.04 / Debian 12 provides VTK version 9.1 but f3d requires VTK 9.2.6 at minimum, so building and installing vtk from source is suggested. Look at Windows build instruction for inspiration if needed.

#### Fedora/Centos/RedHat

```
sudo yum update
sudo dnf install make automake gcc gcc-c++ kernel-devel git git-lfs cmake vtk-devel
```

#### Arch Linux

```
sudo pacman -Syu
sudo pacman -S base-devel git git-lfs cmake vtk nlohmann-json hdf5 netcdf fmt verdict openmpi glew ospray libxcursor
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

### Running the test suite (optional)

```
cmake -DBUILD_TESTING=ON ../src
make
ctest
```

## Windows

### Visual Studio with internal CMake/VCPKG integration

On very recent Visual Studio versions, it's possible to rely on the internal CMake to automatically configure the project
and the internal VCPKG to automatically build the dependencies.

#### Install Dependencies

- Download and install a recent version of [Visual Studio 2022 Community Edition][visual-studio]

> [!WARNING]
> This process has been tested successfully on 17.12 but is known to fail on 17.8

#### Open the project

- Open Visual Studio
- Select `Clone a repository`
- Specify the repository address: `https://github.com/f3d-app/f3d.git` and the local directory of your choice

> [!WARNING]
> The first run will take a while (up to 1 hour) because VTK is compiled twice (in Debug and Release mode)

#### Build and the project

- When the configuration is finished, press `F7` to compile F3D
- Select `f3d.exe (bin\f3d.exe)` in the list of the target and to run F3D

### Command line only

The following steps were tested with Visual Studio Community 2022 using Windows 10, but should also work for other versions.
This guide is relying a lot on terminal command in order to provide easy steps to follow.
Alternatively, it is possible to use Visual Studio directly by opening .sln files and building in Visual Studio.

#### Install Dependencies

- Download and install [git bash for windows][gitforwindows]
- Download and install [cmake][cmake-download]
- Download and install [Visual Studio 2022 Community Edition][visual-studio]

#### Retrieve VTK and F3D sources

- Open git bash

```sh
cd C:
mkdir dev
cd dev
mkdir vtk
cd vtk
git clone --depth 1 --branch v9.5.0 https://gitlab.kitware.com/vtk/vtk.git src
mkdir build
mkdir install
cd ..
mkdir f3d
cd f3d
git clone https://github.com/f3d-app/f3d.git src
mkdir build
```

Note: Resulting directory architecture should look like this:

```
dev
|-- f3d
|   |-- build
|   `-- src
`-- vtk
    |-- build
    |-- install
    `-- src
```

#### Build and install VTK in a dedicated directory

- Open x64 Native Tools Command Prompt for VS 2022

```sh
cd C:\dev\vtk\build
cmake -DCMAKE_INSTALL_PREFIX=../install ../src
cmake --build . -j 16 --config Debug
cmake --install . --config Debug
```

Note: These commands will take a while.

#### Add VTK libraries to the PATH

- Search for "Environment variables" in Windows menu and open associated control panel
- Click on "Environment variables.." button
- Double click on "Path"
- Click on "New" and write "C:\dev\vtk\install\bin"
- Press Ok
- Press Ok
- Press Ok

#### Build F3D

- Open x64 Native Tools Command Prompt for VS 2022

```sh
cd C:\dev\f3d\build
cmake -DVTK_DIR=C:/dev/vtk/install/lib/cmake/vtk-9.5 ../src
cmake --build . --config Debug
```

#### Run

- Open a new file manager window
- Double click on the `C:\dev\f3d\build\bin_Debug\f3d.exe` executable

### Running the test suite (optional)

- Open x64 Native Tools Command Prompt for VS 2022

```
cd C:\dev\f3d\build
cmake -DBUILD_TESTING=ON .
cmake --build . --config Debug
ctest -C Debug
```

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

### Running the test suite (optional)

```
cmake -DBUILD_TESTING=ON ../src
make
ctest
```

[cmake-download]: https://cmake.org/download
[gitforwindows]: https://gitforwindows.org/
[visual-studio]: https://visualstudio.microsoft.com/vs/community/
