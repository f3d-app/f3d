[![CI](https://github.com/f3d-app/f3d/actions/workflows/ci.yml/badge.svg)](https://github.com/f3d-app/f3d/actions/workflows/ci.yml) [![codecov](https://codecov.io/gh/f3d-app/f3d/branch/master/graph/badge.svg?token=siwG82IXK7)](https://codecov.io/gh/f3d-app/f3d) [![Downloads](https://img.shields.io/github/downloads/f3d-app/f3d/total.svg)](https://github.com/f3d-app/f3d/releases)  [![Downloads](https://img.shields.io/reddit/subreddit-subscribers/f3d_app.svg)](https://www.reddit.com/r/f3d_app)

![F3D Logo](./resources/logo.svg)

# F3D - Fast and minimalist 3D viewer

By Michael Migliore and Mathieu Westphal.

F3D (pronounced `/fɛd/`) is a [VTK-based](https://vtk.org) 3D viewer following the [KISS principle](https://en.wikipedia.org/wiki/KISS_principle), so it is minimalist, efficient, has no GUI, has simple interaction mechanisms and is fully controllable using arguments in the command line.

F3D is open-source and cross-platform (tested on Windows, Linux and macOS).
It supports a range of file formats (including animated glTF, stl, step, ply, obj, fbx), and provides numerous rendering and texturing options.

<img src="https://f3d-app.github.io/f3d/gallery/04-f3d.png"  width="640">

*A typical render by F3D*

<img src="https://f3d-app.github.io/f3d/gallery/dota.gif"  width="640">

*Animation of a glTF file within F3D*

<img src="https://f3d-app.github.io/f3d/gallery/directScalars.png"  width="640">

*A direct scalars render by F3D*

# Acknowledgments

F3D was initially created by [Kitware SAS](https://www.kitware.eu/) and is relying on many awesome open source projects, including [VTK](https://vtk.org/), [OCCT](https://dev.opencascade.org/), [Assimp](https://www.assimp.org/) and [Alembic](https://github.com/alembic/alembic).

# How to use

There are 4 main ways to use F3D:

* By running F3D from a terminal with a set of command-line options.
* By running F3D directly and then dragging and dropping files into it to open them.
* By using F3D automatically in the file manager when opening file.
* As a thumbnailer for all supported file formats with certain file managers.

# Installation

You can find the release binary packages for Windows, Linux and macOS on the [Release page](https://github.com/f3d-app/f3d/releases). See the [desktop integration](#desktop-integration) section in order actually integrate the binary release in your desktop.
Alternatively, you can build it yourself following the [build](#Build) guide below.

You can also find packages for the following operating systems:

## OpenSuse

Available in [OpenSuse](https://build.opensuse.org/package/show/graphics/f3d).

## Arch Linux

Available in the [AUR](https://aur.archlinux.org/packages/f3d).

## FreeBSD

Avaiable in [FreshPORTS](https://www.freshports.org/graphics/f3d).

## DragonFly BSD

Available in [DPorts](https://github.com/DragonFlyBSD/DPorts/tree/master/graphics/f3d).

## MacOS Brew

Available in [Homebrew](https://formulae.brew.sh/formula/f3d).

## NixOS

Available in [nixpkgs](https://github.com/NixOS/nixpkgs/blob/master/pkgs/applications/graphics/f3d/default.nix).

## Ubuntu/Debian/Fedora

Available on [OpenSuse OBS](https://build.opensuse.org/package/show/home:AndnoVember:F3D/f3d).

## Flathub

Available in [Flathub](https://flathub.org/apps/details/io.github.f3d_app.f3d).

## Spack

Available in [Spack](https://spack.readthedocs.io/en/latest/package_list.html#f3d).

# Build

## Dependencies

* [CMake](https://cmake.org) >= 3.1.
* [VTK](https://vtk.org) >= 9.0.0 (optionally with raytracing capabilities to enable OSPray rendering).
* A C++17 compiler.
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
* `F3D_MODULE_ALEMBIC`: Support for ABC file format. Requires `Alembic`. Disabled by default.

Then build the software using your build system.

# File formats

Here is the list of supported file formats:

* **.vtk** : the legacy VTK format
* **.vt[p|u|r|i|s|m]** : XML based VTK formats
* **.ply** : Polygon File format
* **.stl** : Standard Triangle Language format
* **.dcm** : DICOM file format
* **.nrrd/.nhrd** : "nearly raw raster data" file format
* **.mhd/.mha** : MetaHeader MetaIO file format
* **.tif/.tiff** : TIFF 2D/3D file format
* **.ex2/.e/.exo/.g** : Exodus 2 file format
* **.gml** : CityGML file format
* **.pts** : Point Cloud file format
* **.step/.stp** : CAD STEP exchange ISO format
* **.iges/.igs** : CAD Initial Graphics Exchange Specification format
* **.abc** : Alembic format
* **.obj** : Wavefront OBJ file format (full scene)
* **.gltf/.glb** : GL Transmission Format (full scene)
* **.3ds** : Autodesk 3D Studio file format (full scene)
* **.wrl** : VRML file format (full scene)
* **.fbx** : Autodesk Filmbox (full scene)
* **.dae** : COLLADA (full scene)
* **.off** : Object File Format (full scene)
* **.dxf** : Drawing Exchange Format (full scene)

# Scene construction

The **full scene** formats (gltf/glb, 3ds, wrl, obj) contain not only *geometry*, but also some scene information like *lights*, *cameras*, *actors* in the scene, as well as *texture* properties.
By default, all this information will be loaded from the file and displayed.
For file formats that do not support it, **a default scene** will be created.

# Options

## Generic Options

Options|Description
------|------
\-\-input=&lt;file&gt;|The *input* file or files to read, can also be provided as a positional argument.
\-\-output=&lt;png file&gt;|Instead of showing a render view and render into it, *render directly into a png file*. When used with ref option, only outputs on failure
\-\-no-background|Output file is saved with a transparent background.
-h, \-\-help|Print *help*.
\-\-verbose|Enable *verbose* mode, providing more information about the loaded data in the console output.
\-\-no-render|Verbose mode without any rendering for the first provided file, to recover information about a file.
\-\-quiet|Enable quiet mode, which superseed any verbose options. No console output will be generated at all.
\-\-version|Show *version* information.
-x, \-\-axis|Show *axes* as a trihedron in the scene.
-g, \-\-grid|Show *a grid* aligned with the XZ plane.
-e, \-\-edges|Show the *cell edges*.
-k, \-\-trackball|Enable trackball interaction.
\-\-progress|Show a *progress bar* when loading the file.
\-\-up|Define the Up direction (default: +Y)
\-\-animation-index|Select the animation to show.<br>Any negative value means all animations.<br>The default scene always has at most one animation.<br>If the option is not specified, the first animation is enabled.
\-\-geometry-only|For certain **full scene** file formats (gltf/glb and obj),<br>reads *only the geometry* from the file and use default scene construction instead.
\-\-dry-run|Do not read the configuration file but consider only the command line options
\-\-config|Read a provided configuration file instead of default one
\-\-font-file|Use the provided FreeType compatible font file to display text.<br>Can be useful to display non-ASCII filenames.

## Material options

Options|Default|Description
------|------|------
-o, \-\-point-sprites||Show sphere *points sprites* instead of the geometry.
\-\-point-size|10.0|Set the *size* of points when showing vertices and point sprites.
\-\-line-width|1.0|Set the *width* of lines when showing edges.
\-\-color=&lt;R,G,B&gt;|1.0, 1.0, 1.0| Set a *color* on the geometry.<br>This only makes sense when using the default scene.
\-\-opacity=&lt;opacity&gt;|1.0|Set *opacity* on the geometry.<br>This only makes sense when using the default scene. Usually used with Depth Peeling option.
\-\-roughness=&lt;roughness&gt;|0.3|Set the *roughness coefficient* on the geometry (0.0-1.0).<br>This only makes sense when using the default scene.
\-\-metallic=&lt;metallic&gt;|0.0|Set the *metallic coefficient* on the geometry (0.0-1.0).<br>This only makes sense when using the default scene.
\-\-hrdi=&lt;file path&gt;||Set the *HDRI* image used to create the environment.<br>The environment act as a light source and is reflected on the material.<br>Valid file format are hdr, png, jpg, pnm, tiff, bmp.
\-\-texture-base-color=&lt;file path&gt;||Path to a texture file that sets the color of the object.
\-\-texture-material=&lt;file path&gt;||Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object.
\-\-texture-emissive=&lt;file path&gt;||Path to a texture file that sets the emitted light of the object.
\-\-emissive-factor=&lt;R,G,B&gt;|1.0, 1.0, 1.0| Emissive factor. This value is multiplied with the emissive color when an emissive texture is present.
\-\-texture-normal=&lt;file path&gt;||Path to a texture file that sets the normal map of the object.
\-\-normal-scale=&lt;normal_scale&gt;|1.0|Normal scale affects the strength of the normal deviation from the normal texture.

## PostFX (OpenGL) options

Options|Description
------|------
-p, \-\-depth-peeling|Enable *depth peeling*. This is a technique used to correctly render translucent objects.
-q, \-\-ssao|Enable *Screen-Space Ambient Occlusion*. This is a technique used to improve the depth perception of the object.
-a, \-\-fxaa|Enable *Fast Approximate Anti-Aliasing*. This technique is used to reduce aliasing.
-t, \-\-tone-mapping|Enable generic filmic *Tone Mapping Pass*. This technique is used to map colors properly to the monitor colors.

## Camera configuration options

Options|Description
------|------
\-\-camera-index|Select the scene camera to use.<br>Any negative value means custom camera.<br>The default scene always has a custom camera.
\-\-camera-position=&lt;X,Y,Z&gt;|The position of the camera. Automatically computed or recovered from the file if not provided.
\-\-camera-focal-point=&lt;X,Y,Z&gt;|The focal point of the camera. Automatically computed or recovered from the file if not provided.
\-\-camera-view-up=&lt;X,Y,Z&gt;|The view up vector of the camera. Will be orthogonalized even when provided. Automatically computed or recovered from the file if not provided.
\-\-camera-view-angle=&lt;angle&gt;|The view angle of the camera, non-zero value in degrees. Automatically computed or recovered from the file if not provided.
\-\-camera-azimuth-angle=&lt;angle&gt;|The azimuth angle of the camera in degrees (default: 0.0).<br>This angle will be applied after setting the position of the camera, focal point of the camera, and view up vector of the camera.
\-\-camera-elevation-angle=&lt;angle&gt;|The elevation angle of the camera in degrees (default: 0.0).<br>This angle will be applied after setting the position of the camera, focal point of the camera, and view up vector of the camera.

## Raytracing options

Options|Default|Description
------|------|------
-r, \-\-raytracing||Enable *OSPRay raytracing*. Requires OSPRay raytracing to be enabled in the linked VTK dependency.
\-\-samples=&lt;samples&gt;|5|The number of *samples per pixel*. It only makes sense with raytracing enabled.
-d, \-\-denoise||*Denoise* the image. It only makes sense with raytracing enabled.

## Scientific visualization options

Options|Default|Description
------|------|------
-s, \-\-scalars=&lt;array_name&gt;||*Color by a specific scalar* array present in the file. If no array_name is provided, one will be picked if any are available. <br>This only makes sense when using the default scene.<br>Use verbose to recover the usable array names.
-y, \-\-comp=&lt;comp_index&gt;|-1|Specify the *component from the scalar* array to color with.<br>Use with the scalar option. -1 means *magnitude*. -2 or the short option, -y, means *direct values*.<br>When using *direct values*, components are used as L, LA, RGB, RGBA values depending on the number of components.
-c, \-\-cells||Specify that the scalar array is to be found *on the cells* instead of on the points.<br>Use with the scalar option.
\-\-range=&lt;min,max&gt;||Set a *custom range for the coloring* by the array.<br>Use with the scalar option.
-b, \-\-bar||Show *scalar bar* of the coloring by array.<br>Use with the scalar option.
\-\-colormap=&lt;color_list&gt;||Set a *custom colormap for the coloring*.<br>This is a list of colors in the format `val1,red1,green1,blue1,...,valN,redN,greenN,blueN`<br>where all values are in the range (0,1).<br>Use with the scalar option.
-v, \-\-volume||Enable *volume rendering*. It is only available for 3D image data (vti, dcm, nrrd, mhd files) and will display nothing with other default scene formats.
-i, \-\-inverse||Inverse the linear opacity function. Only makes sense with volume rendering.

## Testing options

Options|Description
------|------
\-\-ref=&lt;png file&gt;|Reference *image to compare with* for testing purposes. Use with output option to generate new baselines and diff images.
\-\-ref-threshold=&lt;threshold&gt;|*Testing threshold* to trigger a test failure or success.
\-\-interaction-test-record=&lt;log file&gt;|Path to an interaction log file to *record interaction events* to.
\-\-interaction-test-play=&lt;log file&gt;|Path to an interaction log file to *play interactions events* from when loading a file.

## Window options

Options|Default|Description
------|------|------
\-\-bg-color=&lt;R,G,B&gt;|0.2, 0.2, 0.2|Set the window *background color*.<br>Ignored if *hdri* is set.
\-\-resolution=&lt;width,height&gt;|1000, 600|Set the *window resolution*.
-z, \-\-fps||Display a *frame per second counter*.
-n, \-\-filename||Display the *name of the file*.
-m, \-\-metadata||Display the *metadata*.<br>This only makes sense when using the default scene.
-f, \-\-fullscreen||Display in fullscreen.
-u, \-\-blur-background||Blur background.<br>This only makes sense when using a HDRI.

# Rendering precedence

Some rendering options are not compatible between them, here is the precedence order if several are provided:

* Raytracing (`-r`)
* Volume (`-v`)
* Point Sprites (`-o`)

# Interaction

Simple interaction with the displayed data is possible directly within the window. It is as follows:

* *Click and drag* with the *left* mouse button to rotate around the focal point of the camera.
* Hold *Shift* then *Click and drag* horizontally with the *right* mouse button to rotate the HDRI.
* *Click and drag* vertically with the *right* mouse button to zoom in/out.
* *Move the mouse wheel* to zoom in/out.
* *Click and drag* with the *middle* mouse button to translate the camera.
* Drag and drop a file or directory into the F3D window to load it

> Note: When playing an animation with a scene camera, camera interactions are locked.

The coloring can be controlled directly by pressing the following hotkeys:

* `C`: cycle between coloring with array from point data and from cell data.
* `S`: cycle the array to color with.
* `Y`: cycle the component of the array to color with.

See the [coloring cycle](#cycling-coloring) section for more info.

Other options can be toggled directly by pressing the following hotkeys:

* `B`: display of the scalar bar, only when coloring and not using direct scalars.
* `V`: volume rendering.
* `I`: opacity function inversion during volume rendering.
* `O`: point sprites rendering.
* `P`: depth peeling.
* `Q`: Screen-Space Ambient Occlusion.
* `A`: Fast Approximate Anti-Aliasing.
* `T`: tone mapping.
* `E`: the display of cell edges.
* `X`: the trihedral axes display.
* `G`: the XZ grid display.
* `N`: the display of the file name.
* `M`: the display of the metadata if exists.
* `Z`: the display of the FPS counter.
* `R`: raytracing.
* `D`: the denoiser when raytracing.
* `F`: full screen.
* `U`: background blur.
* `K`: trackball interaction mode.

Note that some hotkeys can be available or not depending on the file being loaded and the F3D configuration.

Other hotkeys are available:

* `H`: key to toggle the display of a cheat sheet showing all these hotkeys and their statuses.
* `?`: key to dump camera state to the terminal.
* `ESC`: close the window and quit F3D.
* `ENTER`: reset the camera to its initial parameters.
* `SPACE`: play the animation if any.
* `LEFT`: load the previous file if any.
* `RIGHT`: load the next file if any.
* `UP`: reload the current file.

# Cycling Coloring

When using the default scene, the following hotkeys let you cycle the coloring of the data:

* `C`: cycle between point data and cell data - field data is not supported.
* `S`: cycle the array available on the currently selected data, skipping array not containing numeric data.
It will loop back to not coloring unless using volume rendering.
* `Y`: cycle the component available on the currently selected array, looping to -2 for direct scalars rendering
if the array contains 4 or less components, -1 otherwise.

When changing the array, the component in use will be kept if valid with the new array, if not it will be reset to 0
when coloring with an invalid higher than zero component, and to -1 when using direct scalars rendering with an array
having more than 4 components.

When changing the type of data to color with, the index of the array within the data will be kept if valid
with the new data. If not, it will cycle until a valid array is found. After that, the component will be checked as well.

# Configuration file

Almost all the command-line options can be controlled using a configuration file.
This configuration file uses the "long" version of the options in a JSON
formatted file to provide default values for these options.

These options can be organized by block using a regular expression for each block
in order to provide different default values for the different filetypes.

Using a command-line option will override the corresponding value in the config file.
A typical config file may look like this:

```javascript
{
   ".*": {
       "resolution": "1200,800",
       "bg-color": "0.7,0.7,0.7",
       "color": "0.5,0.1,0.1",
       "fxaa": true,
       "timer": true,
       "progress": true,
       "axis": true,
       "bar": true,
       "verbose": true,
       "roughness": 0.2,
       "grid": true
   },
   ".*vt.": {
       "edges": true
   },
   ".*gl(tf|b)": {
       "raytracing": true,
       "denoise": true,
       "samples": 3
   },
   ".*mhd": {
       "volume": true
   }
}
```
Here, the first block defines a basic configuration with many desired options for all files.
The second block specifies that all files ending with vt., eg: vtk, vtp, vtu, ... will be shown with edges visibility turned on.
The third block specifies raytracing usage for .gltf and .glb files.
The last block specifies that volume rendering should be used with .mhd files.

The following command-line options <br>cannot</br> be set via config file:
`help`, `version`, `config`, `dry-run`, `no-render`, `inputs`, `output` and all testing options.

Boolean options that have been turned on in the configuration file can be turned
off in the command line if needed, eg: `--point-sprites=false`

The configuration file possible locations depends on your operating system.
They are considered in the below order and only the first found will be used.

 * Linux: `/etc/f3d/config.json`, `[install_dir]/config.json`, `${XDG_CONFIG_HOME}/.config/f3d/config.json`, `~/.config/f3d/config.json`
 * Windows: `[install_dir]\config.json`, `%APPDATA%\f3d\config.json`
 * macOS: `/etc/f3d/config.json`, `f3d.app/Contents/Resources/config.json`, `[install_dir]/config.json`, `~/.config/f3d/config.json`

If you are using the releases, a default configuration file is provided when installing F3D.
On Linux, it will be installed in `/etc/f3d/`, on Windows, it will be installed in the install directory, on macOS, it will be installed in the bundle.

# Desktop Integration

F3D can be integrated in the desktop experience.

## Linux

For Linux desktop integration, F3D rely on mime types files as defined by the [XDG standard](https://specifications.freedesktop.org/mime-apps-spec/mime-apps-spec-latest.html), .thumbnailer file as specified [here](https://wiki.archlinux.org/title/File_manager_functionality#Thumbnail_previews) and .desktop file as specified [here](https://wiki.archlinux.org/title/desktop_entries). Many file managers use this mechanism, including nautilus, thunar, pcmanfm and caja.

The simplest way to obtain desktop integration on linux is to use a package for your distribution, or the .deb binary package we provide if compatible with your distribution.
In other cases, the binary archive can be used like this:

0. Make sure ~/.local/bin is part of your PATH
1. Extract F3D archive in a TEMP folder
2. move $TEMP/config.json to ~/.config/f3d/
3. copy $TEMP/* to ~/.local/
4. Update your [mime database](https://linux.die.net/man/1/update-mime-database) pointing to ~/.local/share/mime
5. Update your [desktop database](https://linuxcommandlibrary.com/man/update-desktop-database) pointing to ~/.local/share/application

```bash
tar -xzvf f3d-1.2.0-Linux.tar.gz
cd f3d-1.2.0-Linux
mkdir -p ~/.config/f3d/
mv config.json /.config/f3d/
cp -r ./* ~/.local/
sudo update-mime-database ~/.local/share/mime/
sudo update-desktop-database ~/.local/share/applications
```

If you have any issues, read the [troubleshooting](#Troubleshooting) section.

## Windows

For Windows desktop integration, F3D rely on a registered shell extension.

Using the F3D NSIS installer (.exe) is the simplest way to enable thumbnails and integrate F3D on windows.

It is also possible to do it manually when using the zipped binary release, on installation, just run:

```
cd C:\path\to\f3d\bin\
regsvr32 F3DShellExtension.dll
```

To remove the shell extension, run:

```
cd C:\path\to\f3d\bin\
regsvr32 /u F3DShellExtension.dll
```

## MacOS

There is no support for thumbnails on MacOS, the .dmg binary release should provide automatic file openings.

# Known limitations

* No categorical generic field data rendering support.
* No string array categorical rendering support.
* No support for specifying manual lighting in the default scene.
* Pressing the `z` hotkey to display the FPS timer triggers a double render.
* Multiblock (.vtm, .gml) support is partial, non-surfacic data will be converted into surfaces.
* Animation support with full scene data format require VTK >= 9.0.20201016.
* Full drag and drop support require VTK >= 9.0.20210620
* `Escape` interaction events cannot be recorded.
* Drag and drop interaction cannot be recorded nor played back.
* Volume rendering and HDRI support requires a decent GPU

## Assimp

FBX, DAE, OFF, and DXF file formats rely on [Assimp](https://github.com/assimp/assimp) library. It comes with some known limitations:
- PBR materials are not supported for FBX file format
- Animations are not working very well with Assimp 5.1, it's recommended to use Assimp 5.0
- Some files can be empty, crash, or show artifacts
- DXF support is very limited: only files with polylines and 3D faces are displayed.

## Alembic
ABC file formats rely on [Alembic](https://github.com/alembic/alembic) library. It comes with some known limitations:
- Supports Alembic 1.7 or later
- Supports only simple polygonal geometry
- Does not support ArbGeomParam feature in Alembic
- Does not support Subdivision Meshes
- Does not support Materials

# Troubleshooting

## General
> I have built F3D with raytracing support but the denoiser is not working.

Be sure that VTK has been built with *OpenImageDenoise* support (`VTKOSPRAY_ENABLE_DENOISER` option).

## Linux
> Thumbnails are not working in my file manager.

 * Check that your file manager supports the thumbnailer mechanism.
 * Check that you have updated your mime type database.
 * If all fails, remove your .cache user dir and check that pcmanfm thumbnails are working.
  * If they are working, then it is an issue specific to your file manager.
  * If only a few format have working thumbnails, then it is an issue with mime types
  * If no formats have working thumbnails, then it is an issue with the f3d.thumbnailer file

## Windows
> After installing F3D or registering the shell extension, my explorer is broken

Unregister the shell extension by running:

```
cd C:\path\to\f3d\bin\
regsvr32 /u F3DShellExtension.dll
```

> I use F3D in a VM, the application fails to launch.

OpenGL applications like F3D can have issues when launched from a guest Windows because the access to the GPU is restricted.
You can try to use a software implementation of OpenGL, called [Mesa](https://github.com/pal1000/mesa-dist-win/releases).
 * Download the latest `release-msvc`.
 * copy `x64/OpenGL32.dll` and `x64/libglapi.dll` in the same folder as `f3d.exe`.
 * set the environment variable `MESA_GL_VERSION_OVERRIDE` to 4.5.
 * run `f3d.exe`.

> I run f3d from the command prompt and my Unicode characters are not displayed properly.

Set the codepage to UTF-8, run `chcp 65001`.
