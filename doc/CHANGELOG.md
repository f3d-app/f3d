# Changelog

## v1.3.1

- Fix raytracing support
- Fix macOS window positioning

Contributors: @mwestphal @Meakk

## v1.3.0

Main new features and fixes:
- Introducing an alpha version of the libf3d, see below for more information
- Added a --font-file option
- Added support for Alembic file format (.abc). Geometry only. Thanks a lot @nyue.
- Fixed many HDRI issues
- Fixed an important drag and drop issue on linux

Other fixes:
- Fixed many doc and UI typos
- Fixed shell completion
- Fixed multiple issue with image comparison
- Fixed an issue with --line-width and --point-size with full scene formats
- Fixed an issue with translucent object and RGBA texture
- Fixed issues with a few specific .gltf files
- Fixed a window title issue on MacOS
- Fixed multiple issues with the output window on Windows
- Fixed issues with file association and thumbnails on Windows
- Fixed a small issue with the cheatsheet not appearing in some cases
- Fixed an issue with certain .obj files
- Fixed a clipping issue when setting camera position
- Removed fullscreen hotkey

For F3D developers:
- Updated cxxopts to 3.0.0
- Now using json.hpp for parsing json config file
- Better reproducible build support
- Improved CI with coverage, sanitizer, dedicated actions for dependencies
- Complete rework of the architecture to separate F3D, the application, the libf3d (see below) and a VTKExtensions layer.
- Improved unit testing by adding test in the libf3d and VTKExtensions layer

For libf3d users:
- Introducing an alpha version of the libf3d!
- The libf3d is a C++ library to open and render 3D meshes, it is of course used by F3D and supports python bindings
- More info available in README_libf3d.md

Binary Release Information:
- The binary release is **not** built with raytracing support
- This binary release is built with assimp 5.1.2, animation for assimp formats is not working well

Contributors: @schuhumi @herrriehm @rafcon-dev @mzf-guest @nyue @jpouderoux @Meakk @mwestphal

## v1.2.1

Bug fixes and features:
- Fixed a build issue on MacOS
- Fixed a critical issue with the Windows Thumbnailer that could crash explorer.exe
- Fixed the --quiet option so that it actually quiets VTK errors and warnings
- Fixed an issue with output window popping up at each log on Windows
- Added a `.deb` package for Linux Debian-based distros

For Packagers:
- LFS file have been added in source code release, this seems to be retroactive, so hash of previous releases may have changed
- Flatpack org name has been fixed

Contributors: @Meakk @mwestphal @jpouderoux

## v1.2.0

Main New Features:
- Added STEP and IGES file format support thanks to @Open-Cascade-SAS /[OCCT](https://github.com/Open-Cascade-SAS/OCCT) (thanks @drtrigon)
- Added FBX, DAE, OFF, and DXF file format support thanks to @assimp /[assimp](https://github.com/assimp/assimp)
- Added thumbnail support with many linux file managers
- Added thumbnail support on Windows
- Added desktop environment integration in linux
- Added scalar and scalar component looping

Other New Features:
- Added support for KHR_materials_unlit with glTF files (thanks @spiraloid)
- Added option for selecting camera `--camera-index` (thanks @spiraloid)
- Added coloring the to Windows error output window
- Added a man entry on Linux
- Added a `--config` option to select a config file to load instead of using default location
- Added a `--quiet` option
- Added `--camera-azimuth` and `--camera-elevation` options (thanks @tatsuya-s)
- Added a metainfo.xml file (thanks @kevinsmia1939)

Issue Fixes:
- Fixed an issue with opening files with accented char in the name on Windows (thanks @shankarsivarajan)
- Fixed HDRI orientation with --up option (thanks @truhlikfredy)
- Fixed an issue with point cloud rendering
- Fixed a crash on exit on Windows
- Fixed an issue with fullscreen window size on Windows
- Fixed offscreen rendering (`--output` and `--ref` ) to actually use offscreen rendering
- Fixed a memory leak when no rendering is performed (thanks @CharlesGueunet)
- Fixed a rendering issue with certain GPU drivers
- Fixed tone mapping with background opacity
- Fixed non-working drag and drop implementation in VTK (thanks @msbit)
- Fixed a potential sorting issue when opening a folder
- Fixed a crash with unsupported glTF files in VTK

For Developers:
- Full rework of the CI framework, including coverage report support
- Full rework of the testing framework
- Separation of vtkF3DRenderer in two classes
- Separation of f3d executable into a libf3d library and f3d executable to support windows thumbnails

For Packagers:
- New CMake options to select file to install, all documented and starting with F3D_
- mime types file can be installed, make sure to trigger update-mime-databse
- desktop file can be installed, make sure to trigger update-desktop-database

Packagers: AndnoVember @jokersus @kevinsmia1939 @yurivict @bcdarwin @mzf-guest @Meakk @mwestphal

Binary Release Information:
- The binary release is **not** built with raytracing support
- This binary release is built with assimp 5.1.2, animation for assimp formats is not working well

Contributors: @CharlesGueunet @kevinsmia1939 @mzf-guest @jpouderoux @Meakk @mwestphal

## v1.1.1

A patch release dedicated to package managers that makes F3D v1.1 compatible with vtk v9.1.0

- Fix a render pass build issue with vtk 9.1.0
- Fix a renderer build issue with vtk 9.1.0

Contributors: @Meakk @mwestphal

## v1.1.0

New Important Features:

* **Added direct scalars color rendering mode**, see doc.
* **Added a turntable interactor** and made it default. The previous interactor can still be used using a dedicated option. Thanks @orangebowlerhat @filip.sund and @jjomier for your suggestions.
* **Added animation support** for glTF and Exodus files. Press space for playing the animation.
* Added animation related option, --animation-index, only for glTF when using the full scene.

New Readers and format compatibility features:

* Added skinning and morphing support with glTF files.
* Added TIFF format support. Thanks @proudot for your suggestion.
* Added exodus format support. Thanks @gvernon for your suggestion.
* Added support for OBJ with color directly inside of it instead of using a .mtl file. Thanks @Chenge321 for your suggestion.

Quality of life features:

* Added a hotkey (UP) to reload current file, thanks @caioaao.
* Improved Alpha blending performance, thanks @paul.lafoix.
* Changed the progress bar to a more nice looking one and made it appears only if loading takes time. Thanks @finetjul for the suggestion.
* Improved logging mechanism with color on all OS and a dedicated output window on windows.
* Added a warning when using RayTracing but it is not available.

Fixes:

* Fixed an issue with skybox and edges.
* Fixed a crash when an array had no name.
* Fixed a window naming issue on OSX.
* Fixed a symlink issue.
* Fixed a coloring issue with dataset containing only cell data.

Packaging:
* Upgraded the AUR f3d package to 1.1.0 : https://aur.archlinux.org/packages/f3d.
* Added a Brew f3d 1.1.0 package : https://formulae.brew.sh/formula/f3d.
* FreeBSD now contains a f3d 1.1.0 package, thanks to yuri@freebsd.org.
* NixOS now contains a f3d package, 1.1.0 should come soon, thanks to bcdarwin@gmail.com.

How to support F3D:
* Use the software.
* Share it with anyone interested.
* Star us on github: https://github.com/kitware/F3D.

Note: Binaries have no raytracing support.

Contributors: @Meakk @mwestphal

## v1.0.1

- VisualStudio Runtime is now included into the windows release
- Ensure VTK compatibility
- Improve STL binary reader performances
- Fix default configuration issues
- Add support for Window icon on all OSes

Note: Binaries have no raytracing support.

Contributors: @Meakk @mwestphal

## v1.0.0

- Documentation
  - Online documentation based on Hexo (https://kitware.github.io/F3D/)
- Rendering
  - Add volume rendering for 3D images
  - HDRI support (skyboxes, HDR files, Filmic tone mapping)
  - Point sprites for displaying point clouds
  - Color map customization (default one changed to “inferno”)
- Interface
  - Drag&Drop files support
  - Supports opening of several files (or folder), use left/right arrow to navigate
  - Scalars field handling improvements
  - Cheat Sheet
  - Interactive hotkeys have been reworked
  - File association on Windows and OSX
  - Fullscreen mode
  - No background mode (useful when saving image to a PNG file with alpha channel)
- Command line
  - Camera configuration
  - Metadata (field data) display
  - File name display
  - No render mode (useful to read information in the file)
  - Shell completion (supports bash, zsh, fish)
- New readers
  - CityGML
  - PTS
- Packages
  - Default config provide

Note: Binaries have no raytracing support.

Contributors: @Meakk @mwestphal @hlngrandmontagne Paul Lafoix

## v0.1.1

- Fixes `--version` and `--help` crash

Contributors: @Meakk @mwestphal @jpouderoux

## v0.1.0

First release!

Contributors: @Meakk @mwestphal @jpouderoux
