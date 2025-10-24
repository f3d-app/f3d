# Command line options

F3D behavior can be fully controlled from the command line using the following options.

## Application Options

### `--input=<input file>` (_string_)

The input file or files to read, can also be provided as a positional argument. Support directories as well.

### `--output=<png file>` (_string_)

Instead of showing a render view and render into it, _render directly into a png file_. When used with --ref option, only outputs on failure. If `-` is specified instead of a filename, the PNG file is streamed to the stdout. Can use [template variables](#filename-templating).

### `--no-background` (_bool_, default: `false`)

Use with --output to output a png file with a transparent background.

### `-h`, `--help`

Print _help_ and exit. Ignore `--verbose`.

### `--version`

Show _version_ information and exit. Ignore `--verbose`.

### `--list-readers`

List available _readers_ and exit. Ignore `--verbose`.

### `--force-reader=<reader>` (_string_)

Force a specific [reader](SUPPORTED_FORMATS.md) to be used, disregarding the file extension.

### `--list-bindings`

List available _bindings_ and exit. Ignore `--verbose`.

### `--list-rendering-backends`

List available _rendering backends_ and exit. Ignore `--verbose`.

### `--config=<config file path/name/stem>` (_string_, default: `config`)

Specify the [configuration file](CONFIGURATION_FILE.md) to use. Supports absolute/relative path but also filename/filestem to search for in standard configuration file locations.

### `--no-config` (_bool_, default: `false`)

Do not read any configuration file and consider only the command line options.

### `--no-render` (_bool_, default: `false`)

Do not render anything and quit just after loading the first file, use with --verbose to recover information about a file.

### `--max-size=<size in MiB>` (_int_, default: `-1`)

Prevent F3D to load a file bigger than the provided size in Mib, leave empty for unlimited, useful for thumbnails.

### `--watch` (_bool_, default: `false`)

Watch current file and automatically reload it whenever it is modified on disk. Consider ensuring `--remove-empty-file-groups` is not enabled when using this option.

### `--frame-rate=<fps>` (_double_, default: `30.0`)

Frame rate used to refresh animation and other repeated tasks (watch, UI). Does not impact rendering frame rate.

### `--load-plugins=<paths or names>` (_string_)

List of plugins to load separated with a comma. Official plugins are `alembic`, `assimp`, `draco`, `hdf`, `occt`, `usd`, `vdb`. See [plugins](PLUGINS.md) for more info.

### `--scan-plugins`

Scan standard directories for plugins and display their names, results may be incomplete. See [plugins](PLUGINS.md) for more info.

### `--screenshot-filename=<png file>` (_string_, default: `{app}/{model}_{n}.png`)

Filename to save [screenshots](INTERACTIONS.md#taking-screenshots) to. Can use [template variables](#filename-templating). Supports relative paths [as described](INTERACTIONS.md#taking-screenshots).

### `--rendering-backend=<auto|egl|osmesa|glx|wgl>` (_string_, default: `auto`)

Rendering backend to load, `auto` means to let F3D pick the correct one for you depending on your system capabilities. Use `egl` or `osmesa` on linux to force headless rendering.

### `-D`, `--define=<libf3d.option=value>` (_special_)

A repeatable option to set [libf3d](../libf3d/OPTIONS.md) and [reader](SUPPORTED_FORMATS.md#reader-options) option manually. May trigger unexpected behavior.

### `-R`, `--reset=<libf3d.option>` (_special_)

A repeatable option to reset [libf3d options](../libf3d/OPTIONS.md) manually. Useful when overidding option set in [configuration files](CONFIGURATION_FILE.md).

## General Options

### `--verbose=<[debug|info|warning|error|quiet]>` (_string_, default: `info`)

Set _verbose_ level, in order to provide more information about the loaded data in the output. If no level is provided, assume `debug`. Option parsing may ignore this flag.

### `--progress` (_bool_, default: `false`)

Show a _progress bar_ when loading the file.

### `--animation-progress` (_bool_, default: `false`)

Show a _progress bar_ when playing the animation.

### `--multi-file-mode=<single|all| dir>` (_string_, default: `single`)

When opening multiple files, select if they should be shown all at once (`all`), one by one (`single`), or by directory (`dir`). Configuration files for all loaded files will be used in the order they are provided.

### `--multi-file-regex=<regex>` (_string_)

Regular expression pattern to group files. Captured groups are replaced with `*` so that, for example, the pattern `part(\d+)` would group files `foo-part1.xyz` and `foo-part2.xyz` together as `foo-part*.xyz`.

### `--recursive-dir-add` (_bool_, default: `false`)

When opening a directory, choose if they should be recursively added or not. If not, only the files in the provided directory will be added.

### `--remove-empty-file-groups` (_bool_, default: `false`)

When loading a file group, if they results in an empty scene, remove the file group and load the next file group.

### `--up=<direction>` (_direction_, default: `+Y`)

Define the Up direction.

### `-x`, `--axis` (_bool_, default: `false`)

Show _axes_ as a trihedron in the scene.

### `-g`, `--grid` (_bool_, default: `false`)

Show _a grid_ aligned with the horizontal (orthogonal to the Up direction) plane.

### `--grid-unit=<length>` (_double_)

Set the size of the _unit square_ for the grid. If not set (the default) a suitable value will be automatically computed.

### `--grid-subdivisions=<count>` (_int_, default: `10`)

Set the number of subdivisions for the grid.

### `--grid-color=<color>` (_color_, default: `0,0,0`)

Set the color grid lines.

### `--grid-absolute` (_bool_, default: `false`)

Position the grid at absolute origin instead of below the model.

### `--axes-grid` (_bool_, default: `false`)

Show _axes grid_ in the scene.

### `-e`, `--edges` (_bool_, default: `false`)

Show the _cell edges_.

### `--armature` (_bool_, default: `false`)

Show armature if present (glTF only).

### `--camera-index=<idx>` (_int_)

Select the scene camera to use when available in the file. Automatically computed by default.

### `-k`, `--trackball` (_bool_, default: `false`)

Enable trackball interaction.

### `--invert-zoom` (_bool_, default: `false`)

Invert zoom direction with right mouse click.

### `--animation-autoplay` (_bool_, default: `false`)

Automatically start animation.

### `--animation-indices=<idx1,idx2>` (_vector\<int\>_, default: `0`)

Select the animations to show.
Any negative value all animations.
The default scene always has at most one animation.

### `--animation-speed-factor=<ratio>` (_ratio_, default: `1`)

Set the animation speed factor to slow, speed up or even invert animation time.

### `--animation-time=<time>` (_double_)

Set the animation time to load.

### `--font-file=<font file>` (_path_)

Use the provided FreeType compatible font file to display text.
Can be useful to display non-ASCII filenames.

### `--font-scale=<ratio>` (_ratio_, default: `1.0`)

Scale fonts. Useful for HiDPI displays.

### `--command-script=<command script>` (_script_)

Provide a script file containing a list of [commands](COMMANDS.md) to be executed sequentially.
Allows automation of multiple commands or pre-defined tasks.

### `--backdrop-opacity=<opacity>` (_double_, default: `0.9`)

Set the opacity of the backdrop behind text information such as FPS, filename, metadata or cheatsheet.

## Material options

### `-o`, `--point-sprites` (_bool_, default: `false`)

Show sphere _points sprites_ instead of the geometry.

### `--point-sprites-type=<sphere|gaussian>` (_string_, default: `sphere`)

Set the splat type when showing point sprites.

### `--point-sprites-size=<size>` (_double_, default: `10.0`)

Set the _size_ of point sprites.

### `--point-size=<size>` (_double_)

Set the _size_ of points when showing vertices. Model-specified by default.

### `--line-width=<size>` (_double_)

Set the _width_ of lines when showing edges. Model-specified by default.

### `--backface-type=<visible|hidden>` (_string_)

Set the Backface type. Model-specified by default.

### `--color=<color>` (_color_)

Set a _color_ on the geometry. Multiplied with the base color texture when present.
Model-specified by default.

### `--opacity=<opacity>` (_double_)

Set _opacity_ on the geometry. Multiplied with the base color texture when present.
Model-specified by default. Usually used with `--translucency-support`.

### `--roughness=<roughness>` (_double_)

Set the _roughness coefficient_ on the geometry (0.0-1.0). Multiplied with the material texture when present.
Model-specified by default.

### `--metallic=<metallic>` (_double_)

Set the _metallic coefficient_ on the geometry (0.0-1.0). Multiplied with the material texture when present.
Model-specified by default.

### `--base-ior=<base-ior>` (_double_)

Set the _index of refraction of the base layer_ (1.0-2.5). Model-specified by default.

### `--hdri-file=<HDRI file>` (_path_)

Set the _HDRI_ image that can be used as ambient lighting and skybox.
Valid file format are `.hdr`, `.exr`, `.png`, `.jpg`, `.pnm`, `.tiff`, `.bmp`.
If not set, a default is provided.

### `--hdri-ambient` (_string_)

Light the scene using the _HDRI_ image as ambient lighting.
The environment act as a light source and is reflected on the material.

### `--texture-matcap=<texture file>` (_path_)

Set the texture file to control the material capture of the object. All other model options for surfaces are ignored if this is set. Must be in linear color space.
Model-specified by default.

### `--texture-base-color=<texture file>` (_path_)

Set the texture file to control the color of the object. Please note this will be multiplied with the color and opacity options. Must be in sRGB color space.
Model-specified by default.

### `--texture-material=<texture file>` (_path_)

Set the texture file to control the occlusion, roughness and metallic values of the object. Please note this will be multiplied with the roughness and metallic options, which have impactful default values. To obtain true results, use `--roughness=1` and `--metallic=1`. Must be in linear color space.
Model-specified by default.

### `--texture-emissive=<texture file>` (_path_)

Set the texture file to control the emitted light of the object. Please note this will be multiplied with the emissive factor. Must be in sRGB color space.
Model-specified by default.

### `--emissive-factor=<color>` (_color_)

Set the emissive factor. This value is multiplied with the emissive color when an emissive texture is present.
Model-specified by default.

### `--texture-normal=<texture file>` (_path_)

Set the texture file to control the normal map of the object. Must be in sRGB color space.
Model-specified by default.

### `--normal-scale=<color>` (_double_)

Set the normal scale. This value affects the strength of the normal deviation from the normal texture.
Model-specified by default.

### `--textures-transform=<transform2d>` (_transform2d_)

Set the 2d transform to use for all textures applied to the model.
Importer may set a default value depending on file type. If a default value exists, the default value is multiplied by the provided transform.

## Window options

### `--background-color=<color>` (_color_, default: `0.2, 0.2, 0.2`)

Set the window _background color_.
Ignored if `--hdri-skybox` is enabled.

### `--resolution=<width,height>` (_vector\<double\>_, default: `1000, 600`)

Set the _window resolution_.

### `--position=<x,y>` (_vector\<double\>_)

Set the _window position_ (top left corner) , in pixels, starting from the top left of your screens.

### `-z`, `--fps` (_bool_, default: `false`)

Display a rendering _frame per second counter_.

### `-n`, `--filename` (_bool_, default: `false`)

Display the _name of the file_ on top of the window.

### `-m`, `--metadata` (_bool_, default: `false`)

Display the _metadata_.

### `--hdri-skybox` (_bool_, default: `false`)

Show the HDRI as a skybox. Overrides `--background-color` and `--no-background`.

### `-u`, `--blur-background` (_bool_, default: `false`)

Blur background.
Useful with a HDRI skybox.

### `--blur-coc` (_double_, default: `20`)

Blur circle of confusion radius.

### `--light-intensity` (_double_, default: `1.0`)

_Adjust the intensity_ of every light in the scene.

## Scientific visualization options

### `-s`, `--scalar-coloring` (_bool_, default: `false`)

Enable scalar coloring if present in the file. If `--coloring-array` is not set, the first in alphabetical order will be picked if any are available.

### `--coloring-array=<array_name>` (_string_)

The coloring array name to use when coloring.
Use `--verbose` to recover the usable array names.

### `-y`, `--comp=<comp_index>` (_int_, default: `-1`)

Specify the _component from the scalar_ array to color with.
Use with the scalar option. -1 means _magnitude_. -2 or the short option, -y, means _direct values_.
When using _direct values_, components are used as L, LA, RGB, RGBA values depending on the number of components.

### `-c`, `--cells` (_bool_, default: `false`)

Specify that the scalar array is to be found _on the cells_ instead of on the points.
Use with the scalar option.

### `--range=<min,max>` (_vector\<double\>_)

Set the _coloring range_. Automatically computed by default.
Use with the scalar option.

### `-b`, `--bar` (_bool_, default: `false`)

Show _scalar bar_ of the coloring by array.
Use with the scalar option.

### `--colormap-file=<name>` (_string_)

Set a _colormap file for the coloring_.
See [color maps](COLOR_MAPS.md).
Use with the scalar option.

### `--colormap=<colormap>` (_colormap_)

Set a _custom colormap for the coloring_.See [colormap parsing](PARSING.md#colormap) for details.
Ignored if `--colormap-file` option is specified.
Use with the scalar option.

### `--colormap-discretization=<colors>` (_int_)

Set the number of distinct colors from [1, N] will be used in the colormap. Any values outside the valid range will result in smooth shading.

### `-v`, `--volume` (_bool_, default: `false`)

Enable _volume rendering_. It is only functional for 3D image data (VTKXMLVTI, DICOM, NRRD, MetaImage files) and will display nothing with other formats. It forces coloring.

### `-i`, `--inverse` (_bool_, default: `false`)

Inverse the linear opacity function used for volume rendering.

## Camera configuration options

### `--camera-position=<X,Y,Z>` (_vector\<double\>_)

Set the camera position, overrides `--camera-direction` and `--camera-zoom-factor`.

### `--camera-focal-point=<X,Y,Z>` (_vector\<double\>_)

Set the camera focal point.

### `--camera-view-up=<direction>` (_direction_)

Set the camera view up vector. Will be orthogonalized.

### `--camera-view-angle=<angle>` (_double_)

Set the camera view angle, a strictly positive value in degrees.

### `--camera-direction=<direction>` (_direction_)

Set the camera direction, looking at the focal point.

### `--camera-zoom-factor=<factor>` (_double_)

Set the camera zoom factor relative to the autozoom on data, a strictly positive value.

### `--camera-azimuth-angle=<angle>` (_double_, default: `0.0`)

Apply an azimuth transformation to the camera, in degrees, added after other camera options.

### `--camera-elevation-angle=<angle>` (_double_, default: `0.0`)

Apply an elevation transformation to the camera, in degrees, added after other camera options.

### `--camera-orthographic` (_bool_)

Set the camera to use the orthographic projection. Model-specified by default.

## Raytracing options

### `-r`, `--raytracing` (_bool_, default: `false`)

Enable _OSPRay raytracing_. Requires OSPRay raytracing to be enabled in the linked VTK dependency.

### `--samples=<samples>` (_int_, default: `5`)

Set the number of _samples per pixel_ when using raytracing.

### `-d`, `--denoise` (_bool_, default: `false`)

_Denoise_ the image when using raytracing.

## PostFX (OpenGL) options

### `-p`, `--translucency-support` (_bool_, default: `false`)

Enable _translucency support_. This is a technique used to correctly render translucent objects.

### `-q`, `--ambient-occlusion` (_bool_, default: `false`)

Enable _ambient occlusion_. This is a technique used to improve the depth perception of the object.

### `-a`, `--anti-aliasing` (_bool_, default: `false`)

Enable _anti-aliasing_. This technique is used to reduce aliasing.

### `--anti-aliasing-mode` (_string_, default: `fxaa`)

Anti-aliasing method (`fxaa`: fast, `ssaa`: quality, `taa`: balanced)

> [!WARNING]
> `taa` forces rendering of the scene at regular interval and will introduce ghosting artifacts on animated scenes.
> It also doesn't work with offscreen rendering (when using `--output` option)

### `-t`, `--tone-mapping` (_bool_, default: `false`)

Enable generic filmic _Tone Mapping Pass_. This technique is used to map colors properly to the monitor colors.

### `--final-shader` (_string_)

Add a final shader to the output image. See the [dedicated documentation](FINAL_SHADER.md) for more details.

## Testing options

### `--ref=<png file>` (_string_)

Render and compare with the provided _reference image_, for testing purposes. Use with output option to generate new baselines and diff images.

### `--ref-threshold=<threshold>` (_double_, default: `0.04`)

Set the _comparison threshold_ to trigger a test failure or success. The default (0.04) correspond to almost visually identical images.

### `--interaction-test-record=<log file>` (_string_)

Path to an interaction log file to _record interaction events_ to.

### `--interaction-test-play=<log file>` (_string_)

Path to an interaction log file to _play interactions events_ from when loading a file.

## Rendering options precedence

Some rendering options are not compatible between them, here is the precedence order if several are provided:

- Raytracing (`-r`)
- Volume (`-v`)
- Point Sprites (`-o`)

## Options syntax

To turn on/off boolean options, it is possible to write `--option=true` and `--option=false`, eg `--points-sprites=false`.

As documented, the `--option=value` syntax should be preferred. The syntax `--option value` can have unintended effect with positional arguments.

The `-R` short option has a special syntax: `-Rlibf3d.option` but can also be used with `--reset=libf3d.option`

The `-D/--define` option has a special syntax: `-Dlibf3d.option=value` or `--define=libf3d.option=value`.

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

## HDRI Caches

When using HDRI related options, F3D will create and use a cache directory to store related data in order to speed up rendering.
These cache files can be safely removed at the cost of recomputing them on next use.

The cache directory location is as follows, in order, using the first defined environment variables:

- Windows: `%LOCALAPPDATA%\f3d`
- Linux: `${XDG_CACHE_HOME}/f3d`,`${HOME}/.cache/f3d`
- macOS: `${HOME}/Library/Caches/f3d`
