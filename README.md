![F3D Logo](./resources/logo.svg)

# f3d - Fast and minimalist 3D viewer

By [Kitware SAS](https://www.kitware.eu), 2019

f3d is a [VTK-based](https://vtk.org) 3D viewer following the [KISS principle](https://en.wikipedia.org/wiki/KISS_principle), so it is minimalist, efficient, has no GUI,
has simple interaction mechanisms and is fully controllable using arguments in the command line.

It is open-source and cross-platform (tested on Windows, Linux, and macOS).
It supports a range of file formats, rendering and texturing options.

![F3D Demo](./resources/demo.png)  
*A typical render by f3d*

# Installation

You can find the release binary packages for Windows, Linux, and OSX on the [Release page](https://gitlab.kitware.com/f3d/f3d/-/releases).
Alternatively, you can build it yourself, following the [build guide](#build).

# Build

## Dependencies
* [CMake](https://cmake.org)
* [VTK](https://vtk.org) (optionally with raytracing capabilities to enable OSPray rendering).
* A C++11 compiler
* A CMake-compatible build system (Visual Studio, XCode, Ninja, Make...)

## Configuration and building
Set the following CMake options:
* ` VTK_DIR`: Point to a build or install directory of VTK.
* `BUILD_TESTING`: Optionally, enable the tests.
* `MACOSX_BUILD_BUNDLE`: On MacOS, build a `.app` bundle.
* `WINDOWS_BUILD_WIN32`: On Windows, build a Win32 application (without console).

Then build the software using your build system.

# File formats

Here is the list of supported file formats:
* **.vtk** : the legacy VTK format
* **.vt\*** : XML based VTK formats
* **.ply** : Polygon File format
* **.stl** : Standard Triangle Language format
* **.dcm** : DICOM file format
* **.nrrd/.nhrd** : "nearly raw raster data" file format
* **.mhd/.mha** : MetaHeader MetaIO file format
* **.obj** : Wavefront OBJ file format (full scene)
* **.gltf/.glb** : GL Transmission Format (full scene)
* **.3ds** : Autodesk 3DS Max file format (full scene)
* **.wrl** : VRML file format (full scene)

# Scene construction

The **full scene** formats (gltf/glb, 3ds, wrl, obj) contain not only *geometry*, but also scene information like *lights*, *cameras*, *actors* in the scene and *textures* properties.
By default, all this information will be loaded from the file and displayed.
For file formats that do not support it, **a default scene** will be provided.

# Options

## Generic Options
Options|Description
------|------
--input=\<file\>|The *input* file to read, can also be provided as a positional argument.
-o, --output=\<png file\>|Instead of showing a render view and render into it, *render directly into a png file*.
-h, --help|Print *help*.
-v, --verbose|Enable *verbose* mode.
--version|Show *version* information.
-x, --axis|Show *axes* as a trihedron in the scene.
-g, --grid|Show *a grid* aligned with the XZ plane.
-e, --edges|Show the *cell edges*.
-p, --progress|Show a *progress bar* when loading the file.
-m, --geometry-only|For certain **full scene** file formats (gltf/glb and obj),<br>reads *only the geometry* from the file and use default scene construction instead.

## Material options
Options|Default|Description
------|------|------
--point-size=\<size\>|10.0|Set the *point size*.<br>Points are displayed only if raytracing is activated and the geometry contains vertices,<br>or if the geometry contains only vertices.<br>In this last case, a representation with sphere glyphs is used.<br>This behavior will be changed in the near future.
--color=\<R,G,B\>|1.0, 1.0, 1.0| Set a *color* on the geometry.<br>This only makes sense when using the default scene.
--opacity=\<opacity\>|1.0|Set an *opacity* on the geometry.<br>This only makes sense when using the default scene.
--roughness=\<roughness\>|0.3|Set the *roughness coefficient* on the geometry (0.0-1.0).<br>This only makes sense when using the default scene.
--metallic=\<metallic\>|0.0|Set the *metallic coefficient* on the geometry (0.0-1.0).<br>This only makes sense when using the default scene.

## PostFX (OpenGL) options:
Options|Description
------|------
-d, --depth-peeling|Enable *depth peeling*. This is a technique used to correctly render translucent objects.
-u, --ssao|Enable *Screen-Space Ambient Occlusion*. This is a technique used to improve depth perception of the object.
-f, --fxaa|Enable *Fast Approximate Anti-Aliasing*. This technique is used to reduce aliasing.

## Raytracing options:
Options|Default|Description
------|------|------
-r, --raytracing||Enable *OSPRay raytracing*. Requires OSPray raytracing to be enabled in the linked VTK.
--samples=\<samples\>|5|Number of *samples per pixel*. It only makes sense with raytracing enabled.
-s, --denoise||*Denoise* the image. It only makes sense with raytracing enabled.

## Scientific visualization options:
Options|Default|Description
------|------|------
--scalars=\<array_name\>||*Color by a specific scalar* array present in the file.<br>This only makes sense when using the default scene.<br>Use verbose to recover the usable array names.
--comp=\<comp_index\>|-1|Specify the *component from the scalar* array to color with.<br>Use with the scalar option. Any negative value means *magnitude*.
-c, --cells||Specify that the scalar array is to be found *on the cells* instead of on the points.<br>Use with the scalar option.
--range=\<min,max\>||Set a *custom range for the coloring* by array.<br>Use with the scalar option.
-b, --bar||Show *scalar bar* of the coloring by array.<br>Use with the scalar option.

## Testing options:
Options|Description
------|------
--ref=\<png file\>|Reference *image to compare with* for testing purposes.
--ref-threshold=\<threshold\>|*Testing threshold* to trigger a test failure or success.

## Window options:
Options|Default|Description
------|------|------
--bg-color=\<R,G,B\>|0.2, 0.2, 0.2|Set the window *background color*.
--resolution=\<width,height\>|1000, 600|Set the *window resolution*.
-t, --timer||Display a *frame per second counter*.

# Interaction

Simple interaction with the displayed data is possible directly within the window. It is as follows:
* *Click and drag* with the *left* mouse button to rotate around the focal point of the camera.
* *Click and drag* vertically with the *right* mouse button to zoom in/out.
* *Move the mouse wheel* to zoom in/out.
* *Click and drag* with the *middle* mouse button to translate the camera.
* Press `r` key to reset the camera zoom.
* Press `q` key to close the window and quit f3d.

Some options can be toggled directly using interactions:
* Press `x` key to toggle the trihedral axes display.
* Press `g` key to toggle the XZ grid display.
* Press `e` key to toggle the display of cell edges.
* Press `b` key to toggle the display of the scalar bar, only when coloring with scalars.
* Press `t` key to toggle the display of the FPS counter.

# Limitations

* No support for animation.
* No support for specifying manual lighting in the default scene.
* No support for volume rendering
* Cannot open files from Finder on OSX
