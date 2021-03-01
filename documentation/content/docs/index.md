![F3D Logo](logo.png)

# F3D - Fast and minimalist 3D viewer

By [Kitware SAS](https://www.kitware.eu), 2019

F3D (pronounced `/fɛd/`) is a [VTK-based](https://vtk.org) 3D viewer following the [KISS principle](https://en.wikipedia.org/wiki/KISS_principle), so it is minimalist, efficient, has no GUI, has simple interaction mechanisms and is fully controllable using arguments in the command line.

It is open-source and cross-platform (tested on Windows, Linux, and macOS).
It supports a range of file formats, rendering and texturing options.

![F3D Demo](../gallery/04-f3d.png)
*A typical render by F3D*

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
Options &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|Description
------|------
\-\-input=&lt;file&gt;|The *input* file or files to read, can also be provided as a positional argument.
\-\-output=&lt;png file&gt;|Instead of showing a render view and render into it, *render directly into a png file*.
\-\-no-background|Output file is saved with a transparent background.
-h, \-\-help|Print *help*.
\-\-verbose|Enable *verbose* mode.
\-\-no-render|Verbose mode without any rendering for the first provided file, to recover information about a file.
\-\-version|Show *version* information.
-x, \-\-axis|Show *axes* as a trihedron in the scene.
-g, \-\-grid|Show *a grid* aligned with the XZ plane.
-e, \-\-edges|Show the *cell edges*.
-k, \-\-trackball|Enable trackball interaction.
\-\-progress|Show a *progress bar* when loading the file.
\-\-animation-index|Select the animation to show.<br>Any negative value means all animations.<br>The default scene always has a single animation if any.
\-\-geometry-only|For certain **full scene** file formats (gltf/glb and obj),<br>reads *only the geometry* from the file and use default scene construction instead.
\-\-up|Define the Up direction (default: +Y)
\-\-dry-run|Do not read the configuration file but consider only the command line options

## Material options
Options&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|Default|Description
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
\-\-texture-emissive=&lt;file path&gt;||Path to a texture file that sets the emited light of the object.
\-\-emissive-factor=&lt;R,G,B&gt;|1.0, 1.0, 1.0| Emissive factor. This value is multiplied with the emissive color when an emissive texture is present.
\-\-texture-normal=&lt;file path&gt;||Path to a texture file that sets the normal map of the object.
\-\-normal-scale=&lt;normal_scale&gt;|1.0|Normal scale affects the strength of the normal deviation from the normal texture.

## PostFX (OpenGL) options:
Options &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|Description
------|------
-p, \-\-depth-peeling|Enable *depth peeling*. This is a technique used to correctly render translucent objects.
-q, \-\-ssao|Enable *Screen-Space Ambient Occlusion*. This is a technique used to improve the depth perception of the object.
-a, \-\-fxaa|Enable *Fast Approximate Anti-Aliasing*. This technique is used to reduce aliasing.
-t, \-\-tone-mapping|Enable generic filmic *Tone Mapping Pass*. This technique is used to map colors properly to the monitor colors.

## Camera configuration options:
Options &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|Description
------|------
\-\-camera-index|Select the scene camera to use.<br>Any negative value means custom camera.<br>The default scene always has a custom camera.
\-\-camera-position=&lt;X,Y,Z&gt;|The position of the camera. Automaticaly computed or recovered from the file if not provided.
\-\-camera-focal-point=&lt;X,Y,Z&gt;|The focal point of the camera. Automaticaly computed or recovered from the file if not provided.
\-\-camera-view-up=&lt;X,Y,Z&gt;|The focal point of the camera. Will be orthogonalized even when provided. Automaticaly computed or recovered from the file if not provided.
\-\-camera-view-angle=&lt;angle&gt;|The view angle of the camera, non-zero value in degrees. Automaticaly computed or recovered from the file if not provided.

## Raytracing options:
Options &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|Default|Description
------|------|------
-r, \-\-raytracing||Enable *OSPRay raytracing*. Requires OSPRay raytracing to be enabled in the linked VTK.
\-\-samples=&lt;samples&gt;|5|The number of *samples per pixel*. It only makes sense with raytracing enabled.
-d, \-\-denoise||*Denoise* the image. It only makes sense with raytracing enabled.

## Scientific visualization options:
Options &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|Default|Description
------|------|------
-s, \-\-scalars=&lt;array_name&gt;||*Color by a specific scalar* array present in the file. If no array_name is provided, one will be picked if any are available. <br>This only makes sense when using the default scene.<br>Use verbose to recover the usable array names.
-y, \-\-comp=&lt;comp_index&gt;|-1|Specify the *component from the scalar* array to color with.<br>Use with the scalar option. -1 means *magnitude*. -2 or the short option, -y, means *direct values*.<br>When using *direct values*, components are used as L, LA, RGB, RGBA values depending on the number of components.
-c, \-\-cells||Specify that the scalar array is to be found *on the cells* instead of on the points.<br>Use with the scalar option.
\-\-range=&lt;min,max&gt;||Set a *custom range for the coloring* by the array.<br>Use with the scalar option.
-b, \-\-bar||Show *scalar bar* of the coloring by array.<br>Use with the scalar option.
\-\-colormap=&lt;color_list&gt;||Set a *custom colormap for the coloring*.<br>This is a list of colors in the format `val1,red1,green1,blue1,...,valN,redN,greenN,blueN`<br>where all values are in the range (0,1).<br>Use with the scalar option.
-v, \-\-volume||Enable *volume rendering*. It is only available for 3D image data (vti, dcm, nrrd, mhd files) and will display nothing with other default scene formats.
-i, \-\-inverse||Inverse the linear opacity function. Only makes sense with volume rendering.

## Testing options:
Options|Description
------|------
\-\-ref=&lt;png file&gt;|Reference *image to compare with* for testing purposes.
\-\-ref-threshold=&lt;threshold&gt;|*Testing threshold* to trigger a test failure or success.

## Window options:
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
Some rendering options are not compatible between them, here is the precedence order if several are defined:
- Raytracing (`-r`)
- Volume (`-v`)
- Point Sprites (`-o`)

# Interaction

Simple interaction with the displayed data is possible directly within the window. It is as follows:
* *Click and drag* with the *left* mouse button to rotate around the focal point of the camera.
* Hold *Shift* then *Click and drag* horizontally with the *right* mouse button to rotate the HDRI.
* *Click and drag* vertically with the *right* mouse button to zoom in/out.
* *Move the mouse wheel* to zoom in/out.
* *Click and drag* with the *middle* mouse button to translate the camera.
* Drag and Drop a file or directory into the F3D window to load it

> Note: When playing an animation with a scene camera, camera interactions are locked.

The coloring can be controlled directly using hotkeys:
* Press `C` key to cycle between coloring with array from point data and from cell data.
* Press `S` key to cycle the array to color with.
* Press `Y` key to cycle the component of the array to color with.
See the coloring cycle section for more info.

Other options can be toggled directly using hotkeys:
* Press `B` key to toggle the display of the scalar bar, only when coloring and not using direct scalars.
* Press `V` key to toggle volume rendering.
* Press `I` key to toggle opacity function inversion during volume rendering.
* Press `O` key to toggle point sprites rendering.
* Press `P` key to toggle depth peeling.
* Press `Q` key to toggle Screen-Space Ambient Occlusion.
* Press `A` key to toggle Fast Approximate Anti-Aliasing.
* Press `T` key to toggle tone mapping.
* Press `E` key to toggle the display of cell edges.
* Press `X` key to toggle the trihedral axes display.
* Press `G` key to toggle the XZ grid display.
* Press `N` key to toggle the display of the file name.
* Press `M` key to toggle the display of the metadata if exists.
* Press `Z` key to toggle the display of the FPS counter.
* Press `R` key to toggle raytracing.
* Press `D` key to toggle the denoiser when raytracing.
* Press `F` key to toggle full screen.
* Press `U` key to toggle background blur.
* Press `K` key to toggle trackball interaction mode.
Certains hotkeys can be available or not depending of the file being loaded and F3D configuration.

Other hotkeys are available:
* Press `H` key to toggle the display of a cheat sheet showing all these hotkeys and their statuses.
* Press `?` key to dump camera state to the terminal.
* Press `ESC` key to close the window and quit F3D.
* Press `ENTER` key to reset the camera to its inital parameters.
* Press `SPACE` key to play the animation if any.
* Press `LEFT` to load the previous file if any.
* Press `RIGHT` to load the next file if any.
* Press `UP` to reload the current file.

# Cycling Coloring
When using the default scene, using the 'C', 'S' and 'Y' hotkeys let you cycle the coloring of the data.
'C' let you cycle between point data and cell data, field data is not supported.
'S' let you cycle the array available on the currently selected data, skipping array not containing numeric data.
It will loop back to not coloring unless using volume rendering.
'Y' let you cycle the component available on the currently selected array, looping to -2 for direct scalars rendering
if the array contains 4 or less components, -1 otherwise.

When changing the array, the component in use will be kept if valid with the new array, if not it will be reset to 0
when coloring with an invalid higher than zero component, and to -1 when using direct scalars rendering with an array
having more than 4 components.

When changing the type of data to color with, the index of the array within the data will be kept if valid
with the new data. If not, it will cycle until a valid array is found. After that, the component will be checked as well.
