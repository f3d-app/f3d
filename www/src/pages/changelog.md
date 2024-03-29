---
sidebar_position: 7
---

# Changelog

## Ongoing development

For F3D users:
- Added a new option `--point-type` used to specify how to display points
- Add support for 3D Gaussians Splatting in binary .splat format

For libf3d users:
- Added a new option `render.splat-type` used to specify how to display points (only if `model.point-sprites.enable` is true)

## v2.3.0

For F3D users:
 - Added `--animation-autoplay` option (libf3d: `scene.animation.autoplay`) to automatically start animation on file load
 - Added an experimental `vdb` plugin in order to read .vdb files using OpenVDB
 - Added an `usd` plugin in order to read .usd files using OpenUSD
 - Added .3mf and .x files support in the `assimp` plugin
 - Added a small margin between the model and the grid to avoid potential z-fighting
 - Added VTK output to the log when using `--verbose=debug` for easier debug
 - Reworked the `alembic` plugin to read Texture Coordinates and Normals for .abc files
 - Improved the reading of EXR files using multi-threading for better performance when loading HDRIs
 - Improved the documentation on https://f3d.app
 - Improved general logging in order to make it easier to understand issues with file and options
 - Fixed potential crashes when reading `assimp` plugin file formats
 - Fixed `--grid-absolute` for off-center models
 - Fixed an issue with thumbnail generation showing the log window
 - Fixed many interaction issues on non-Qwerty keyboard layout
 - Fixed handling of filenames containing commas
 - Fixed an issue on Windows where the log window would appear on top of F3D window
 - Fixed crashes with specific .obj file
 - Fixed an issue with incorrect return code when using headless mode or incorrect options
 - Fixed dependency issue with the headless .deb binary release
 - Fixed a crash when using a file with more than 254 bones (Requires OpenGL 4.3)
 - Fixed an issue with Unicode filenames on the title bar on Windows
 - Fixed an issue where focal point picking would generate error messages
 - Changed `--verbose` into a string based option, eg: `--verbose=quiet` or `--verbose=debug`. `--verbose` is still supported.
 - Changed `--no-render` behavior so that it doesn't impact verbosity anymore
 - Changed the default configuration file so that translucency support is enabled by default
 - Deprecated `--quiet`, use `--verbose=quiet` instead
 - Deprecated `--input`, use positional argument instead

For libf3d users:
 - Added `scene.animation.autoplay` option
 - Added a `f3d::image::saveBuffer` API to save an image as a file format in memory
 - Fixed the default name of the window

For F3D packagers:
 - Added a new optional dependency, OpenUSD. Enable with `F3D_PLUGIN_BUILD_USD` CMake option
 - Added a new optional dependency through VTK, OpenVDB. Enable with `F3D_PLUGIN_BUILD_VDB` CMake option
 - Added a new CMake option to control the link against libatomic. Control with `F3D_LINUX_LIBRARY_LINK_ATOMIC`
 - Added two new packages to the list, OpenMandriva and Mageia

## v2.2.1

For F3D users:
 - Added a native Apple Silicon release package
 - Added new options `hdri-file`, `hdri-ambient`, `hdri-skybox` to have more control on the HDRI behavior
 - Added a default HDRI used when providing no `hdri-file`
 - Added bindings to toggle HDRI ambient lighting (`F`) and HDRI skybox (`J`)
 - Added bindings to move the camera to standard locations: `1`: Front, `3`: Right, `7`: Top, `9`: Isometric
 - Added [Open CASCADE BRep format](https://dev.opencascade.org/doc/overview/html/specification__brep_format.html) to the OCCT plugin.
 - Fixed an infamous issue when the Windows thumbnailer always steal the focus
 - Fixed an issue with the binary release when opening draco files
 - Fixed an issue with matcap textures
 - Fixed an issue with HDRI lighting
 - Fixed an issue with HDRI lighting when dropping HDRI after a model
 - Fixed an issue where invalid option would not show on Windows
 - Fixed an issue where previously loaded file would stay visible
 - Fixed an issue when loading invalid EXR file as HDRI
 - Fixed cheatsheet menu rendering under 'Drop File Instructor'
 - Fixed raytracing rendering issues
 - Improved cheatsheet menu contrast for any background color
 - Improved overall text contrast for any background color
 - Improved performance when changing model and using a HDRI
 - Improved HDRI cache performance
 - Deprecated `hdri` option

For libf3d users:
 - Reworked image API to support many file formats to read (EXR, HDR) and write (PNG, JPG, TIF, BMP)
 - Added `render.hdri.file`, `render.hdri.ambient`, `render.background.skybox` options.
 - Deprecated `render.background.hdri` in favor of new options above
 - Deprecated previous image API
 - Add `loader::loadGeometry` allowing loading geometry from memory buffers

For developers:
 - Added a deprecation framework
 - Removed `F3D_TESTING_ENABLE_HDRI_TESTS` cmake option and merged it with `F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS`
 - Added a Getting Started guide for beginners developers

For F3D packagers:
 - Fixed compatibility with FreeBSD

## v2.1.0

For F3D users:
 - Reworked Animation support and added many animation related options
 - Added `--camera-direction` and `--camera-zoom-factor` and use it in default config file
 - Added `--texture-matcap` option to load a MatCap texture
 - Added support for opening HDRI background with Drop
 - Added .exr support for HDRI and textures
 - Improved HDRI caching performance
 - Added a EGL headless binary release
 - Added typo suggestion when failing to use the right option name
 - Added `--invert-zoom` option (libf3d: `interactor.invert-zoom`) to invert zoom direction with right-click on mouse
 - Added support for keeping camera position when reloading a file with `UP` key
 - Added a `--grid-absolute` option (libf3d: `render.grid.absolute`) to position the grid instead of being below the model
 - Added a "Drop Zone" when starting F3D without loading a file
 - Added coloring support for STEP files in binary release
 - Fixed an issue on Windows where the error window would popup even with `--quiet`
 - Fixed an issue with 16 bit and 32 bit textures with assimp plugin
 - Fixed an issue on Windows where errors where not visible
 - Fixed an HDRI issue on reload
 - Fixed a crash when with the progress bar on reloading a file
 - Fixed an issue where certain options from config file would be ignored
 - Fixed a crash with many GLTF files when an array does not have a name
 - Fixed an issue where dropping a duplicate file would change the loaded file instead of not doing anything
 - Fixed an issue when loading an unsupported file
 - Fixed an issue with VRML files
 - Improved config file for .ply files
 - Improved config file regex so that they are case-insensitive
 - Improved `--up` option to support no sign and better validation
 - Improved F3D version reporting about itself and VTK (`--version`)
 - Improved documentation, testing, help
 - Excluded TIFF format from Windows Thumbnailer
 - Changed the automatic camera position to a version more close to the data
 - Reduced size of binary release by a factor of two

For developers:
 - Added vcpkg support for dependency installation

For F3D packagers:
 - Added a `F3D_PLUGIN_OCCT_COLORING_SUPPORT` CMake variable to control if this feature is compiled, default is ON
 - Added `SONAME` support
 - Added a `F3D_PLUGINS_INSTALL_DIR` CMake variable to control where to install plugins
 - Added a new module: `F3D_MODULE_EXR` that depends on openexr
 - Added support to use external `cxxopts` and external `nlohmann-json`
 - Fixed a link issue with VTK modules
 - Fixed a `@loader_path` issue on MacOS
 - Added support for `CMAKE_INSTALL_*DIR` variables on installation
 - Added support for building against a EGL enabled VTK
 - Fixed a compilation issue with VTK master

Contributors: @DeveloperPaul123 @topazus @bkmgit @snoyer @Meakk @mwestphal

## v2.0.0

For F3D users:
- Removed `--fullscreen` options completely
- Added a `--position` option to set the position of the F3D window
- Added a `--light-intensity` option and associated bindings
- Improved fish and zsh completion
- Added a new interaction to center the camera on the point under the cursor using *middle* mouse button
- Added HDRi caching for improved performance
- Added scene bounding box to the dump interaction
- Reworked lighting for fullscene format files
- Improved grid rendering and customization greatly with new options
- Improved HDRi blur performance and customization
- Added support for reading config file directory instead of a single file
- Changed config file location on Windows
- PostFX option names have been changed
- Added an interaction (down arrow) to load all files from the directory containing the current file
- Added a `--group-geometries` option to load multiple default scene file into a single render window

For libf3d users:
- Reworked loader API from scratch
- Fixed an issue with engine creation in the Python bindings
- Fixed an issue with image Python bindings
- Improved libf3d examples
- Added experimental javascript bindings based on webassembly/emscriptem
- Added dynamic support for all `model` options
- Added plugin SDK to create your own plugins for any file format

For F3D packagers:
- Added plugins support for true optional dependencies
- Added a `F3D_APPLICATION_LINK_FILESYSTEM` CMake option to control if F3D should link explicitly against libstdc++fs
- Removed most of `F3D_INSTALL_` options with the switch to a component-based approach
- Removed installation of useless static libs from VTKExtensions (needs VTK master)
- Fixed RPATH of F3D for system using lib64 instead of lib
- Many CMake option name changed
- Fixed data license to use only permissive license for F3D testing data
- Removed packaging support from F3D

Contributors: @snoyer @mzf-guest @CharlesGueunet @myd7349 @allemangD @Meakk @mwestphal

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
