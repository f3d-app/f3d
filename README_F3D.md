# F3D - Fast and minimalist 3D viewer

By Michael Migliore and Mathieu Westphal.

F3D (pronounced `/fɛd/`) is a [VTK-based](https://vtk.org) 3D viewer following the [KISS principle](https://en.wikipedia.org/wiki/KISS_principle), so it is minimalist, efficient, has no GUI, has simple interaction mechanisms and is fully controllable using arguments in the command line.

F3D is open-source and cross-platform (tested on Windows, Linux and macOS).
It supports a range of file formats (including animated glTF, stl, step, ply, obj, fbx), and provides numerous rendering and texturing options.

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
\-\-color=&lt;R,G,B&gt;|1.0, 1.0, 1.0| Set a *color* on the geometry. Multiplied with the base color texture when present. <br>This only makes sense when using the default scene.
\-\-opacity=&lt;opacity&gt;|1.0|Set *opacity* on the geometry. Multiplied with the base color texture when present. <br>This only makes sense when using the default scene. Usually used with Depth Peeling option.
\-\-roughness=&lt;roughness&gt;|0.3|Set the *roughness coefficient* on the geometry (0.0-1.0). Multiplied with the material texture when present. <br>This only makes sense when using the default scene.
\-\-metallic=&lt;metallic&gt;|0.0|Set the *metallic coefficient* on the geometry (0.0-1.0). Multiplied with the material texture when present. <br>This only makes sense when using the default scene.
\-\-hrdi=&lt;file path&gt;||Set the *HDRI* image used to create the environment.<br>The environment act as a light source and is reflected on the material.<br>Valid file format are hdr, png, jpg, pnm, tiff, bmp.
\-\-texture-base-color=&lt;file path&gt;||Path to a texture file that sets the color of the object. Please note this will be multiplied with the color and opacity options.
\-\-texture-material=&lt;file path&gt;||Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object. Please note this will be multiplied with the roughness and metallic options, which have impactful default values. To obtain true results, use `--roughness=1 --metallic=1`.
\-\-texture-emissive=&lt;file path&gt;||Path to a texture file that sets the emitted light of the object. Please note this will be multiplied with the emissive factor.
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
\-\-camera-index|Select the scene camera to use when available in the file.<br>Any negative value means automatic camera.<br>The default scene always uses automatic camera.
\-\-camera-position=&lt;X,Y,Z&gt;|The position of the camera.
\-\-camera-focal-point=&lt;X,Y,Z&gt;|The focal point of the camera.
\-\-camera-view-up=&lt;X,Y,Z&gt;|The view up vector of the camera. Will be orthogonalized even when provided.
\-\-camera-view-angle=&lt;angle&gt;|The view angle of the camera, non-zero value in degrees.
\-\-camera-azimuth-angle=&lt;angle&gt;|Apply an azimuth transformation to the camera, in degrees (default: 0.0).
\-\-camera-elevation-angle=&lt;angle&gt;|Apply an elevation transformation to the camera, in degrees (default: 0.0).

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
* `U`: background blur.
* `K`: trackball interaction mode.

Note that some hotkeys can be available or not depending on the file being loaded and the F3D configuration.

Other hotkeys are available:

* `H`: key to toggle the display of a cheat sheet showing all these hotkeys and their statuses.
* `?`: key to print scene description to the terminal.
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
