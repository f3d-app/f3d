<!-- {% assign model = "{{model}}" %} set variable to prevent Jekyll from swallowing `{{model}}` when expanding template variables -->

# Command line options

F3D behavior can be fully controlled from the command line using the following options.

## Application Options

Options|Type<br>Default|Description
------|------|------
\-\-input=\<input file\>|string<br>-|The input file or files to read, can also be provided as a positional argument.
\-\-output=\<png file\>|string<br>-|Instead of showing a render view and render into it, *render directly into a png file*. When used with \-\-ref option, only outputs on failure. If `-` is specified instead of a filename, the PNG file is streamed to the stdout. Can use [template variables](#filename-templating).
\-\-no-background|bool<br>false|Use with \-\-output to output a png file with a transparent background.
-h, \-\-help||Print *help* and exit. Ignore `--verbose`.
\-\-version||Show *version* information and exit. Ignore `--verbose`.
\-\-list-readers||List available *readers* and exit. Ignore `--verbose`.
\-\-list-bindings||List available *bindings* and exit. Ignore `--verbose`.
\-\-list-rendering-backends||List available *rendering backends* and exit. Ignore `--verbose`.
\-\-config=\<config file path/name/stem\>|string<br>config|Specify the [configuration file](CONFIGURATION_FILE.md) to use. Supports absolute/relative path but also filename/filestem to search for in standard configuration file locations.
\-\-no-config|bool<br>false|Do not read any configuration file and consider only the command line options.
\-\-no-render|bool<br>false|Do not render anything and quit just after loading the first file, use with \-\-verbose to recover information about a file.
\-\-max-size=\<size in MiB\>|int<br>-1|Prevent F3D to load a file bigger than the provided size in Mib, leave empty for unlimited, useful for thumbnails.
\-\-watch|bool<br>false|Watch current file and automatically reload it whenever it is modified on disk.
\-\-frame-rate=\<fps\>|double<br>30.0|Frame rate used to refresh animation and other repeated tasks (watch, UI). Does not impact rendering frame rate.
\-\-load-plugins=\<paths or names\>|string<br>-|List of plugins to load separated with a comma. Official plugins are `alembic`, `assimp`, `draco`, `exodus`, `occt`, `usd`, `vdb`. See [plugins](PLUGINS.md) for more info.
\-\-scan-plugins||Scan standard directories for plugins and display their names, results may be incomplete. See [plugins](PLUGINS.md) for more info.
\-\-screenshot-filename=\<png file\>|string<br>`{app}/{model}_{n}.png`|Filename to save [screenshots](INTERACTIONS.md#taking-screenshots) to. Can use [template variables](#filename-templating). Supports relative paths [as described](INTERACTIONS.md#taking-screenshots).
\-\-rendering-backend=\<auto\|egl\|osmesa\|glx\|wgl\>|string<br>auto|Rendering backend to load, `auto` means to let F3D pick the correct one for you depending on your system capabilities. Use `egl` or `osmesa` on linux to force headless rendering.

## General Options

Options|Type<br>Default|Description
------|------|------
\-\-verbose=\<[debug\|info\|warning\|error\|quiet]\>|string<br>info| Set *verbose* level, in order to provide more information about the loaded data in the console output. If no level is provided, assume `debug`. Option parsing may ignore this flag.
\-\-progress|bool<br>false|Show a *progress bar* when loading the file.
\-\-animation-progress|bool<br>false|Show a *progress bar* when playing the animation.
\-\-multi-file-mode=\<single|all\>|string<br>single|When opening multiple files, select if they should be grouped (`all`) or alone (`single`). Configuration files for all loaded files will be used in the order they are provided.
\-\-up=\<[+\|-][X\|Y\|Z]\>|string<br>+Y|Define the Up direction.
-x, \-\-axis|bool<br>false|Show *axes* as a trihedron in the scene.
-g, \-\-grid|bool<br>false|Show *a grid* aligned with the horizontal (orthogonal to the Up direction) plane.
\-\-grid\-unit=\<length\>|double<br>-|Set the size of the *unit square* for the grid. If not set (the default) a suitable value will be automatically computed.
\-\-grid\-subdivisions=\<count\>|int<br>10|Set the number of subdivisions for the grid.
\-\-grid\-color=\<color\>|vector\<double\><br>(0,0,0)|Set the color grid lines.
-e, \-\-edges|bool<br>false|Show the *cell edges*.
\-\-armature|bool<br>false|Show armature if present (glTF only).
\-\-camera-index=\<idx\>|int<br>-|Select the scene camera to use when available in the file. Automatically computed by default.
-k, \-\-trackball|bool<br>false|Enable trackball interaction.
\-\-animation-autoplay|bool<br>false|Automatically start animation.
\-\-animation-index=\<idx\>|int<br>0|Select the animation to show.<br>Any negative value means all animations (glTF only).<br>The default scene always has at most one animation.
\-\-animation-speed-factor=\<factor\>|ratio<br>1|Set the animation speed factor to slow, speed up or even invert animation time.
\-\-animation-time=\<time\>|double<br>-|Set the animation time to load.
\-\-font-file=\<font file\>|string<br>-|Use the provided FreeType compatible font file to display text.<br>Can be useful to display non-ASCII filenames.
\-\-font-scale=\<scale\>|ratio<br>1.0|Scale fonts. 
\-\-command-script=\<command script\>|script<br>-|Provide a script file containing a list of commands to be executed sequentially.<br>Allows automation of multiple commands or pre-defined tasks.

## Material options

Options|Type<br>Default|Description
------|------|------
-o, \-\-point-sprites|bool<br>false|Show sphere *points sprites* instead of the geometry.
\-\-point-sprites-type=\<sphere\|gaussian\>|string<br>sphere|Set the splat type when showing point sprites.
\-\-point-sprites-size=\<size\>|double<br>10.0|Set the *size* of point sprites.
\-\-point-size=\<size\>|double<br>-|Set the *size* of points when showing vertices. Model specified by default.
\-\-line-width=\<size\>|double<br>-|Set the *width* of lines when showing edges. Model specified by default.
\-\-backface-type=\<visible\|hidden\>|string<br>-|Set the Backface type. Model specified by default.
\-\-color=\<R,G,B\>|vector\<double\><br>-| Set a *color* on the geometry. Multiplied with the base color texture when present. <br>Model specified by default.
\-\-opacity=\<opacity\>|double<br>-|Set *opacity* on the geometry. Multiplied with the base color texture when present. <br>Model specified by default. Usually used with Depth Peeling option.
\-\-roughness=\<roughness\>|double<br>-|Set the *roughness coefficient* on the geometry (0.0-1.0). Multiplied with the material texture when present. <br>Model specified by default.
\-\-metallic=\<metallic\>|double<br>-|Set the *metallic coefficient* on the geometry (0.0-1.0). Multiplied with the material texture when present. <br>Model specified by default.
\-\-hdri-file=\<HDRI file\>|string<br>-|Set the *HDRI* image that can be used as ambient lighting and skybox.<br>Valid file format are hdr, exr, png, jpg, pnm, tiff, bmp. <br> If not set, a default is provided.
\-\-hdri-ambient|string<br>-|Light the scene using the *HDRI* image as ambient lighting.<br>The environment act as a light source and is reflected on the material.
\-\-texture-matcap=\<texture file\>|string<br>-|Set the texture file to control the material capture of the object. All other model options for surfaces are ignored if this is set. Must be in linear color space. <br>Model specified by default.
\-\-texture-base-color=\<texture file\>|string<br>-|Set the texture file to control the color of the object. Please note this will be multiplied with the color and opacity options. Must be in sRGB color space. <br>Model specified by default.
\-\-texture-material=\<texture file\>|string<br>-|Set the texture file to control the occlusion, roughness and metallic values of the object. Please note this will be multiplied with the roughness and metallic options, which have impactful default values. To obtain true results, use \-\-roughness=1 \-\-metallic=1. Must be in linear color space. <br>Model specified by default.
\-\-texture-emissive=\<texture file\>|string<br>-|Set the texture file to control the emitted light of the object. Please note this will be multiplied with the emissive factor.  Must be in sRGB color space. <br>Model specified by default.
\-\-emissive-factor=\<R,G,B\>|vector\<double\><br>-|Set the emissive factor. This value is multiplied with the emissive color when an emissive texture is present. <br>Model specified by default.

## Window options

Options|Type<br>Default|Description
------|------|------
\-\-background-color=\<R,G,B\>|vector\<double\><br>0.2, 0.2, 0.2|Set the window *background color*.<br>Ignored if *hdri* is set.
\-\-resolution=\<width,height\>|vector\<double\><br>1000, 600|Set the *window resolution*.
\-\-position=\<x,y\>|vector\<double\><br>-|Set the *window position* (top left corner) , in pixels, starting from the top left of your screens.
-z, \-\-fps|bool<br>false|Display a rendering *frame per second counter*.
-n, \-\-filename|bool<br>false|Display the *name of the file* on top of the window.
-m, \-\-metadata|bool<br>false|Display the *metadata*.
\-\-hdri-skybox|bool<br>false|Show the HDRI as a skybox. Overrides \-\-background-color and \-\-no-background.
-u, \-\-blur-background|bool<br>false|Blur background.<br>Useful with a HDRI skybox.
\-\-blur-coc|double<br>20|Blur circle of confusion radius.
\-\-light-intensity|double<br>1.0|*Adjust the intensity* of every light in the scene.

## Scientific visualization options

Options|Type<br>Default|Description
------|------|------
-s, \-\-scalar-coloring|bool<br>false|Enable scalar coloring if present in the file. If `--coloring-array` is not set, the first in alphabetical order will be picked if any are available.
\-\-coloring-array=\<array_name\>|string<br>-|The coloring array name to use when coloring.<br>Use \-\-verbose to recover the usable array names.
-y, \-\-comp=\<comp_index\>|int<br>-1|Specify the *component from the scalar* array to color with.<br>Use with the scalar option. -1 means *magnitude*. -2 or the short option, -y, means *direct values*.<br>When using *direct values*, components are used as L, LA, RGB, RGBA values depending on the number of components.
-c, \-\-cells|bool<br>false|Specify that the scalar array is to be found *on the cells* instead of on the points.<br>Use with the scalar option.
\-\-range=\<min,max\>|vector\<double\><br>-|Set the *coloring range*. Automatically computed by default.<br>Use with the scalar option.
-b, \-\-bar|bool<br>false|Show *scalar bar* of the coloring by array.<br>Use with the scalar option.
\-\-colormap\-file=\<name\>|string<br>-|Set a *colormap file for the coloring*.<br>See [color maps](COLOR_MAPS.md).<br>Use with the scalar option.
\-\-colormap=\<color_list\>|string<br>-|Set a *custom colormap for the coloring*.<br>This is a list of colors in the format `val1,red1,green1,blue1,...,valN,redN,greenN,blueN`<br>where all values are in the range (0,1).<br>Ignored if `--colormap-file` option is specified.<br>Use with the scalar option.
-v, \-\-volume|bool<br>false|Enable *volume rendering*. It is only available for 3D image data (vti, dcm, nrrd, mhd files) and will display nothing with other formats. It forces coloring.
-i, \-\-inverse|bool<br>false|Inverse the linear opacity function used for volume rendering.

## Camera configuration options

Options|Type<br>Default|Description
------|------|------
\-\-camera-position=\<X,Y,Z\>|vector\<double\><br>-|Set the camera position, overrides --camera-direction and camera-zoom-factor.
\-\-camera-focal-point=\<X,Y,Z\>|vector\<double\><br>-|Set the camera focal point.
\-\-camera-view-up=\<X,Y,Z\>|vector\<double\><br>-|Set the camera view up vector. Will be orthogonalized.
\-\-camera-view-angle=\<angle\>|double<br>-|Set the camera view angle, a strictly positive value in degrees.
\-\-camera-direction=\<X,Y,Z\>|vector\<double\><br>-|Set the camera direction, looking at the focal point.
\-\-camera-zoom-factor=\<factor\>|double<br>-|Set the camera zoom factor relative to the autozoom on data, a strictly positive value.
\-\-camera-azimuth-angle=\<angle\>|double<br>0.0|Apply an azimuth transformation to the camera, in degrees, added after other camera options.
\-\-camera-elevation-angle=\<angle\>|double<br>0.0|Apply an elevation transformation to the camera, in degrees, added after other camera options.
\-\-camera-orthographic|bool<br>-|Set the camera to use the orthographic projection. Model specified by default.

## Raytracing options

Options|Type<br>Default|Description
------|------|------
-r, \-\-raytracing|bool<br>false|Enable *OSPRay raytracing*. Requires OSPRay raytracing to be enabled in the linked VTK dependency.
\-\-samples=\<samples\>|int<br>5|Set the number of *samples per pixel* when using raytracing.
-d, \-\-denoise|bool<br>false|*Denoise* the image when using raytracing.

## PostFX (OpenGL) options

Options|Type<br>Default|Description
------|------|------
-p, \-\-translucency-support|bool<br>false|Enable *translucency support*. This is a technique used to correctly render translucent objects.
-q, \-\-ambient-occlusion|bool<br>false|Enable *ambient occlusion*. This is a technique used to improve the depth perception of the object.
-a, \-\-anti-aliasing|bool<br>false|Enable *anti-aliasing*. This technique is used to reduce aliasing.
-t, \-\-tone-mapping|bool<br>false|Enable generic filmic *Tone Mapping Pass*. This technique is used to map colors properly to the monitor colors.
\-\-final-shader|string<br>-|Add a final shader to the output image. See the [dedicated documentation](FINAL_SHADER.md) for more details.

## Testing options

Options|Type<br>Default|Description
------|------|------
\-\-ref=\<png file\>|string<br>-|Render and compare with the provided *reference image*, for testing purposes. Use with output option to generate new baselines and diff images.
\-\-ref-threshold=\<threshold\>|double<br>0.04|Set the *comparison threshold* to trigger a test failure or success. The default (0.04) correspond to almost visually identical images.
\-\-interaction-test-record=\<log file\>|string<br>-|Path to an interaction log file to *record interaction events* to.
\-\-interaction-test-play=\<log file\>|string<br>-|Path to an interaction log file to *play interactions events* from when loading a file.

## Rendering options precedence

Some rendering options are not compatible between them, here is the precedence order if several are provided:

* Raytracing (`-r`)
* Volume (`-v`)
* Point Sprites (`-o`)

## Options syntax

To turn on/off boolean options, it is possible to write `--option=true` and `--option=false`, eg `--points-sprites=false`.

As documented, only the `--option=value` syntax is supported. The syntax `--option value` is not supported.

All options are parsed according to their type, see the [parsing documentation](PARSING.md) for more details.

## Filename templating

The destination filename used by `--output` or to save screenshots using `--screenshot-filename` can use the following template variables:

- `{app}`: application name (ie. `F3D`)
- `{version}`: application version (eg. `2.4.0`)
- `{version_full}`: full application version (eg. `2.4.0-abcdefgh`)
- `{model}`: current model filename without extension (eg. `foo` for `/home/user/foo.glb`)
- `{model.ext}`: current model filename with extension (eg. `foo.glb` for `/home/user/foo.glb`)
- `{model_ext}`: current model filename extension (eg. `glb` for `/home/user/foo.glb`)
- `{date}`: current date in YYYYMMDD format
- `{date:format}`: current date as per C++'s `std::put_time` format
- `{n}`: auto-incremented number to make filename unique (up to 1000000)
- `{n:2}`, `{n:3}`, ...: zero-padded auto-incremented number to make filename unique (up to 1000000)
- variable names can be escaped by doubling the braces (eg. use `{{model}}.png` to output `{model}.png` without the model name being substituted)

For example the screenshot filename is configured as `{app}/{model}_{n}.png` by default, meaning that, assuming the model `hello.glb` is being viewed,
consecutive screenshots are going to be saved as `F3D/hello_1.png`, `F3D/hello_2.png`, `F3D/hello_3.png`, ...

Model related variables will be replaced by `no_file` if no file is loaded and `multi_file` if multiple files are loaded using the `multi-file-mode` option.
