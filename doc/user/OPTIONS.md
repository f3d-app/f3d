# Command line options

F3D behavior can be fully controlled from the command line using the following options.

## Applicative Options

Options|Default|Description
------|------|------
\-\-output=\<png file\>||Instead of showing a render view and render into it, *render directly into a png file*. When used with \-\-ref option, only outputs on failure.
\-\-no-background||Use with \-\-output to output a png file with a transparent background.
-h, \-\-help||Print *help* and exit. Ignore `--verbose`.
\-\-version||Show *version* information and exit. Ignore `--verbose`.
\-\-readers-list||List available *readers* and exit. Ignore `--verbose`.
\-\-config=\<config file path/name/stem\>|config|Specify the [configuration file](CONFIGURATION_FILE.md) to use. Supports absolute/relative path but also filename/filestem to search for in standard configuration file locations.
\-\-dry-run||Do not read any configuration file and consider only the command line options.
\-\-no-render||Do not render anything and quit just after loading the first file, use with \-\-verbose to recover information about a file.
\-\-max-size=\<size in MiB\>|-1|Prevent F3D to load a file bigger than the provided size in Mib, negative value means unlimited, useful for thumbnails.
\-\-load-plugins=\<paths or names\>||List of plugins to load separated with a comma. Official plugins are `alembic`, `assimp`, `draco`, `exodus`, `occt`, `usd`, `vdb`. See [usage](USAGE.md) for more info.

## General Options

Options|Default|Description
------|------|------
\-\-verbose=\<[debug\|info\|warning\|error\|quiet]\>|info| Set *verbose* level, in order to provide more information about the loaded data in the console output. If no level is providen, assume `debug`.
\-\-progress||Show a *progress bar* when loading the file.
\-\-geometry-only||For certain **full scene** file formats (gltf/glb and obj),<br>reads *only the geometry* from the file and use default scene construction instead.
\-\-group-geometries||When opening multiple files, show them all in the same scene.<br>Force geometry-only. The configuration file for the first file will be loaded.
\-\-up=\<[+\|-][X\|Y\|Z]\>|+Y|Define the Up direction.
-x, \-\-axis||Show *axes* as a trihedron in the scene.
-g, \-\-grid||Show *a grid* aligned with the horizontal (orthogonal to the Up direction) plane.
\-\-grid\-unit=\<length\>||Set the size of the *unit square* for the grid. If set to non-positive (the default) a suitable value will be automatically computed.
\-\-grid\-subdivisions=\<count\>||Set the number of subdivisions for the grid.
-e, \-\-edges||Show the *cell edges*.
\-\-camera-index=\<idx\>|-1|Select the scene camera to use when available in the file.<br>Any negative value means automatic camera.<br>The default scene always uses automatic camera.
-k, \-\-trackball||Enable trackball interaction.
\-\-animation-autoplay||Automatically start animation.
\-\-animation-index=\<idx\>|0|Select the animation to show.<br>Any negative value means all animations (glTF only).<br>The default scene always has at most one animation.
\-\-animation-speed-factor=\<factor\>|1|Set the animation speed factor to slow, speed up or even invert animation time.
\-\-animation-frame-rate=\<factor\>|60|Set the animation frame rate used when playing animation interactively.
\-\-font-file=\<font file\>||Use the provided FreeType compatible font file to display text.<br>Can be useful to display non-ASCII filenames.

## Material options

Options|Default|Description
------|------|------
-o, \-\-point-sprites||Show sphere *points sprites* instead of the geometry.
\-\-point-size=\<size\>|10.0|Set the *size* of points when showing vertices and point sprites.
\-\-line-width=\<size\>|1.0|Set the *width* of lines when showing edges.
\-\-color=\<R,G,B\>|1.0, 1.0, 1.0| Set a *color* on the geometry. Multiplied with the base color texture when present. <br>Requires a default scene.
\-\-opacity=\<opacity\>|1.0|Set *opacity* on the geometry. Multiplied with the base color texture when present. <br>Requires a default scene. Usually used with Depth Peeling option.
\-\-roughness=\<roughness\>|0.3|Set the *roughness coefficient* on the geometry (0.0-1.0). Multiplied with the material texture when present. <br>Requires a default scene.
\-\-metallic=\<metallic\>|0.0|Set the *metallic coefficient* on the geometry (0.0-1.0). Multiplied with the material texture when present. <br>Requires a default scene.
\-\-hdri-file=\<HDRI file\>||Set the *HDRI* image that can be used as ambient lighting and skybox.<br>Valid file format are hdr, exr, png, jpg, pnm, tiff, bmp. <br> If not set, a default is provided.
\-\-hdri-ambient||Light the scene using the *HDRI* image as ambient lighting.<br>The environment act as a light source and is reflected on the material.
\-\-texture-matcap=\<texture file\>||Set the texture file to control the material capture of the object. All other model options for surfaces are ignored if this is set. Must be in linear color space.
\-\-texture-base-color=\<texture file\>||Set the texture file to control the color of the object. Please note this will be multiplied with the color and opacity options. Must be in sRGB color space.
\-\-texture-material=\<texture file\>||Set the texture file to control the occlusion, roughness and metallic values of the object. Please note this will be multiplied with the roughness and metallic options, which have impactful default values. To obtain true results, use \-\-roughness=1 \-\-metallic=1. Must be in linear color space.
\-\-texture-emissive=\<texture file\>||Set the texture file to control the emitted light of the object. Please note this will be multiplied with the emissive factor.  Must be in sRGB color space.
\-\-emissive-factor=\<R,G,B\>|1.0, 1.0, 1.0|Set the emissive factor. This value is multiplied with the emissive color when an emissive texture is present.

## Window options

Options|Default|Description
------|------|------
\-\-bg-color=\<R,G,B\>|0.2, 0.2, 0.2|Set the window *background color*.<br>Ignored if *hdri* is set.
\-\-resolution=\<width,height\>|1000, 600|Set the *window resolution*.
\-\-position=\<x,y\>||Set the *window position* (top left corner) , in pixels, starting from the top left of your screens.
-z, \-\-fps||Display a *frame per second counter*.
-n, \-\-filename||Display the *name of the file* on top of the window.
-m, \-\-metadata||Display the *metadata*.<br>Empty without a default scene.
\-\-hdri-skybox||Show the HDRI as a skybox. Overrides \-\-bg-color and \-\-no-background.
-u, \-\-blur-background||Blur background.<br>Useful with a HDRI skybox.
\-\-blur-coc|20|Blur circle of confusion radius.
\-\-light-intensity|1.0|*Adjust the intensity* of every light in the scene.

## Scientific visualization options

Options|Default|Description
------|------|------
-s, \-\-scalars=\<array_name\>||Specify an array to *Color* with if present in the file. If no array_name is provided, the first in alphabetical order will be picked if any are available. <br>Requires a default scene.<br>Use \-\-verbose to recover the usable array names.
-y, \-\-comp=\<comp_index\>|-1|Specify the *component from the scalar* array to color with.<br>Use with the scalar option. -1 means *magnitude*. -2 or the short option, -y, means *direct values*.<br>When using *direct values*, components are used as L, LA, RGB, RGBA values depending on the number of components.
-c, \-\-cells||Specify that the scalar array is to be found *on the cells* instead of on the points.<br>Use with the scalar option.
\-\-range=\<min,max\>||Set a *custom range for the coloring* by the array.<br>Use with the scalar option.
-b, \-\-bar||Show *scalar bar* of the coloring by array.<br>Use with the scalar option.
\-\-colormap=\<color_list\>||Set a *custom colormap for the coloring*.<br>This is a list of colors in the format `val1,red1,green1,blue1,...,valN,redN,greenN,blueN`<br>where all values are in the range (0,1).<br>Use with the scalar option.
-v, \-\-volume||Enable *volume rendering*. It is only available for 3D image data (vti, dcm, nrrd, mhd files) and will display nothing with other formats.
-i, \-\-inverse||Inverse the linear opacity function used for volume rendering.

## Camera configuration options

Options|Default|Description
------|------|------
\-\-camera-position=\<X,Y,Z\>||Set the camera position, overrides --camera-direction and camera-zoom-factor.
\-\-camera-focal-point=\<X,Y,Z\>||Set the camera focal point.
\-\-camera-view-up=\<X,Y,Z\>||Set the camera view up vector. Will be orthogonalized.
\-\-camera-view-angle=\<angle\>||Set the camera view angle, a strictly positive value in degrees.
\-\-camera-direction=\<X,Y,Z\>||Set the camera direction, looking at the focal point.
\-\-camera-zoom-factor=\<factor\>||Set the camera zoom factor relative to the autozoom on data, a strictly positive value.
\-\-camera-azimuth-angle=\<angle\>|0.0|Apply an azimuth transformation to the camera, in degrees, added after other camera options.
\-\-camera-elevation-angle=\<angle\>|0.0|Apply an elevation transformation to the camera, in degrees, added after other camera options.

## Raytracing options

Options|Default|Description
------|------|------
-r, \-\-raytracing||Enable *OSPRay raytracing*. Requires OSPRay raytracing to be enabled in the linked VTK dependency.
\-\-samples=\<samples\>|5|Set the number of *samples per pixel* when using raytracing.
-d, \-\-denoise||*Denoise* the image when using raytracing.

## PostFX (OpenGL) options

Options|Description
------|------
-p, \-\-translucency-support|Enable *translucency support*. This is a technique used to correctly render translucent objects.
-q, \-\-ambient-occlusion|Enable *ambient occlusion*. This is a technique used to improve the depth perception of the object.
-a, \-\-anti-aliasing|Enable *anti-aliasing*. This technique is used to reduce aliasing.
-t, \-\-tone-mapping|Enable generic filmic *Tone Mapping Pass*. This technique is used to map colors properly to the monitor colors.

## Testing options

Options|Default|Description
------|------|------
\-\-ref=\<png file\>||Render and compare with the provided *reference image*, for testing purposes. Use with output option to generate new baselines and diff images.
\-\-ref-threshold=\<threshold\>|50|Set the *comparison threshold* to trigger a test failure or success. The default (50) correspond to almost visually identical images.
\-\-interaction-test-record=\<log file\>||Path to an interaction log file to *record interaction events* to.
\-\-interaction-test-play=\<log file\>||Path to an interaction log file to *play interactions events* from when loading a file.

## Rendering options precedence

Some rendering options are not compatible between them, here is the precedence order if several are provided:

* Raytracing (`-r`)
* Volume (`-v`)
* Point Sprites (`-o`)

## Options syntax

The `--options=value` syntax is used everywhere in this documentation, however, the syntax `--options value` can also be used, with the exception of options that have implicit values,
`--verbose`, `--comp` and `--scalars`.
