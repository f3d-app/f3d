# Command line options

F3D behavior can be fully controlled from the command line using the following options.

## Application Options

### `--input=<input file>` (_string_)

The input file or files to read, can also be provided as a positional argument. Support directories as well.
If `-` is specified instead of a filename, the file will be streamed from the stdin, which will hang until a stream is provided.

### `--output=<png file>` (_string_)

Instead of showing a render view and render into it, _render directly into a png file_. When used with --ref option, only outputs on failure. If `-` is specified instead of a filename, the PNG file is streamed to the stdout. Can use [template variables](#filename-templating). When using the `{frame}` variable, multiple animation frames are exported (see [Exporting animation frames](05-ANIMATIONS.md#exporting-animation-frames)).

### `--no-background` (_bool_, default: `false`)

Use with --output to output a png file with a transparent background.

### `-h`, `--help`

Print _help_ and exit. Ignore `--verbose`.

### `--version`

Show _version_ information and exit. Ignore `--verbose`.

### `--list-readers`

List available _readers_ and exit. Ignore `--verbose`.

### `--force-reader=<reader>` (_string_)

Force a specific [reader](02-SUPPORTED_FORMATS.md) to be used, disregarding the file extension and file content.

### `--list-bindings`

List available _bindings_ and exit. Ignore `--verbose`.

### `--list-rendering-backends`

List available _rendering backends_ and exit. Ignore `--verbose`.

### `--config=<config file path/name/stem>` (_string_, default: `config`)

Specify the [configuration file](06-CONFIGURATION_FILE.md) to use. Supports absolute/relative path but also filename/filestem to search for in standard configuration file locations.

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

List of plugins to load separated with a comma. Official plugins are `alembic`, `assimp`, `draco`, `hdf`, `occt`, `pdal`, `usd`, `vdb`, `webifc`. See [plugins](12-PLUGINS.md) for more info.

### `--plugins-path=<path>` (_string_)

Supplementary path to specify where to load plugins from, on top of [default paths](12-PLUGINS.md#supported-plugins).

### `--scan-plugins`

Scan standard directories for plugins and display their names, results may be incomplete. See [plugins](12-PLUGINS.md) for more info.

### `--screenshot-filename=<png file>` (_string_, default: `{app}/{model}_{n}.png`)

Filename to save [screenshots](04-INTERACTIONS.md#taking-screenshots) to. Can use [template variables](#filename-templating). Supports relative paths [as described](04-INTERACTIONS.md#taking-screenshots).

### `--rendering-backend=<auto|egl|osmesa|glx|wgl>` (_string_, default: `auto`)

Rendering backend to load, `auto` means to let F3D pick the correct one for you depending on your system capabilities. Use `egl` or `osmesa` on linux to force headless rendering.

### `-D`, `--define=<libf3d.option=value>` (_special_)

A repeatable option to set [libf3d](../libf3d/03-OPTIONS.md) and [reader](02-SUPPORTED_FORMATS.md#reader-options) option manually. May trigger unexpected behavior.

### `-R`, `--reset=<libf3d.option>` (_special_)

A repeatable option to reset [libf3d options](../libf3d/03-OPTIONS.md) manually. Useful when overidding option set in [configuration files](06-CONFIGURATION_FILE.md).

## General Options

### `--verbose=<[debug|info|warning|error|quiet]>` (_string_, default: `info`, implicit: `debug`)

Set _verbose_ level, in order to provide more information about the loaded data in the output. Error reporting during option parsing may ignore this flag.

### `--loading-progress` (_bool_, default: `false`)

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

| +Y                     | +Z                     |
| ---------------------- | ---------------------- |
| ![](./images/up_y.png) | ![](./images/up_z.png) |

### `-x`, `--axis` (_bool_, default: `false`)

Show _axes_ as a trihedron in the scene.

#### compare

| OFF                                       | ON                        |
| ----------------------------------------- | ------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/axis_on.png) |

### `--x-color` (_color_, default: `f3d_red`)

Set the X axis color in both widget and grid.

#### compare

| Default                         | Yellow                           |
| ------------------------------- | -------------------------------- |
| ![](./images/color_default.png) | ![](./images/x_color_yellow.png) |

### `--y-color` (_color_, default: `f3d_green`)

Set the Y axis color in both widget and grid.

#### compare

| Default                         | Yellow                           |
| ------------------------------- | -------------------------------- |
| ![](./images/color_default.png) | ![](./images/y_color_yellow.png) |

### `--z-color` (_color_, default: `f3d_blue`)

Set the Z axis color in both widget and grid.

#### compare

| Default                         | Yellow                           |
| ------------------------------- | -------------------------------- |
| ![](./images/color_default.png) | ![](./images/z_color_yellow.png) |

### `-g`, `--grid` (_bool_, default: `false`)

Show _a grid_ aligned with the horizontal (orthogonal to the Up direction) plane.

#### compare

| OFF                                       | ON                        |
| ----------------------------------------- | ------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/grid_on.png) |

### `--grid-unit=<length>` (_double_)

Set the size of the _unit square_ for the grid. If not set (the default) a suitable value will be automatically computed.

#### compare

| Default                             | ON                            |
| ----------------------------------- | ----------------------------- |
| ![](./images/grid_unit_default.png) | ![](./images/grid_unit_2.png) |

### `--grid-subdivisions=<count>` (_int_, default: `10`)

Set the number of subdivisions for the grid.

#### compare

| 10 (default)                                | 4                                     |
| ------------------------------------------- | ------------------------------------- |
| ![](./images/grid_subdivisions_default.png) | ![](./images/grid_subdivisions_4.png) |

### `--grid-color=<color>` (_color_, default: `0,0,0`)

Set the color grid lines.

#### compare

| Default                              | Cyan                              |
| ------------------------------------ | --------------------------------- |
| ![](./images/grid_color_default.png) | ![](./images/grid_color_cyan.png) |

### `--grid-reflection=<strength>` (_double_, default: `0`)

Set the reflection strength on the grid.

#### compare

| OFF                                   | ON                                   |
| ------------------------------------- | ------------------------------------ |
| ![](./images/grid_reflection_off.png) | ![](./images/grid_reflection_on.png) |

### `--grid-absolute` (_bool_, default: `false`)

Position the grid at absolute origin instead of below the model.

#### compare

| OFF                                 | ON                                 |
| ----------------------------------- | ---------------------------------- |
| ![](./images/grid_absolute_off.png) | ![](./images/grid_absolute_on.png) |

### `--axes-grid` (_bool_, default: `false`)

Show _axes grid_ in the scene.

#### compare

| OFF                                       | ON                             |
| ----------------------------------------- | ------------------------------ |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/axes_grid_on.png) |

### `-e`, `--edges` (_bool_, default: `false`)

Show the _cell edges_.

#### compare

| OFF                                       | ON                         |
| ----------------------------------------- | -------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/edges_on.png) |

### `--armature` (_bool_, default: `false`)

Show armature if present.

#### compare

| Disabled                       | Enable                        |
| ------------------------------ | ----------------------------- |
| ![](./images/armature_off.png) | ![](./images/armature_on.png) |

### `--camera-index=<idx>` (_int_)

Select the scene camera to use when available in the file. Automatically computed by default.

### `-k`, `--interaction-style=<default|trackball|2d>` (_string_, default: `default`)

Set the interaction style. `default` uses standard 3D interaction, `trackball` enables trackball interaction, `2d` enables 2D interaction mode (pan/zoom only, no rotation). In `2d` mode, the default camera zoom factor is `1.0` (no padding) instead of `0.9`.

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

#### compare

| 1                              | 2                              |
| ------------------------------ | ------------------------------ |
| ![](./images/font_scale_1.png) | ![](./images/font_scale_2.png) |

### `--font-color=<color>` (_color_, default: `0.957,0.957,0.957`)

Set font color. Defaults to #F4F4F4(F3DWhite).

#### compare

| Default                              | Red (1,0,0)                      |
| ------------------------------------ | -------------------------------- |
| ![](./images/font_color_default.png) | ![](./images/font_color_red.png) |

### `--dpi-aware` (_bool_, default: `false`)

Scale the _font scale_ and _resolution_ by the display scaling factor.
Only supported on Windows platform.

### `--command-script=<command script>` (_script_)

Provide a script file containing a list of [commands](07-COMMANDS.md) to be executed sequentially.
Allows automation of multiple commands or pre-defined tasks.

### `--backdrop-color=<color>` (_color_, default: `f3d_black`)

Set the color of the backdrop behind text information such as FPS, filename, metadata or cheatsheet.

#### compare

| Default (black)                          | Navy                                  |
| ---------------------------------------- | ------------------------------------- |
| ![](./images/backdrop_color_default.png) | ![](./images/backdrop_color_navy.png) |

### `--backdrop-opacity=<opacity>` (_double_, default: `0.9`)

Set the opacity of the backdrop behind text information such as FPS, filename, metadata or cheatsheet.

#### compare

| Default (0.9)                              | 0.2                                    |
| ------------------------------------------ | -------------------------------------- |
| ![](./images/backdrop_opacity_default.png) | ![](./images/backdrop_opacity_0.2.png) |

## Material options

### `--normal-glyphs` (_bool_, default: `false`)

Display arrows that show vertex normals.

#### compare

| OFF                                       | ON                                 |
| ----------------------------------------- | ---------------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/normal_glyphs_on.png) |

### `--normal-glyphs-scale` (_ratio_, default: `1.0`)

Adjusts the scales of normal glyphs.

#### compare

| 0.3                                       | 1                                       |
| ----------------------------------------- | --------------------------------------- |
| ![](./images/normal_glyphs_scale_0.3.png) | ![](./images/normal_glyphs_scale_1.png) |

### `-o`, `--point-sprites=<none|sphere|gaussian|circle|stddev|bound|cross>` (_string_, default: `none`, implicit: `sphere`)

Select _points sprites_ types to show instead of the geometry.

#### compare

| none                                      | sphere                                 |
| ----------------------------------------- | -------------------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/point_sprites_sphere.png) |

### `--point-sprites-size=<size>` (_double_, default: `10.0`)

Set the _size_ of point sprites.

#### compare

| 0.5                                    | 0.2                                      |
| -------------------------------------- | ---------------------------------------- |
| ![](./images/point_sprites_sphere.png) | ![](./images/point_sprites_size_0.2.png) |

### `--point-sprites-absolute-size` (_bool_, default: `false`)

Do not scale the point sprites size by the scene bounding box.

### `--point-size=<size>` (_double_)

Set the _size_ of points when showing vertices. Model-specified by default.

#### compare

| Default (10)                    | 5                              |
| ------------------------------- | ------------------------------ |
| ![](./images/point_size_10.png) | ![](./images/point_size_5.png) |

### `--line-width=<size>` (_double_)

Set the _width_ of lines when showing edges. Model-specified by default.

#### compare

| 1                              | 4                              |
| ------------------------------ | ------------------------------ |
| ![](./images/line_width_1.png) | ![](./images/line_width_4.png) |

### `--backface-type=<visible|hidden>` (_string_)

Set the Backface type. Model-specified by default.

#### compare

| visible                                 | hidden                                 |
| --------------------------------------- | -------------------------------------- |
| ![](./images/backface_type_visible.png) | ![](./images/backface_type_hidden.png) |

### `--color=<color>` (_color_)

Set a _color_ on the geometry. Multiplied with the base color texture when present.
Model-specified by default. Overridden by `--scalar-coloring`.

#### compare

| red                         | blue                         |
| --------------------------- | ---------------------------- |
| ![](./images/color_red.png) | ![](./images/color_blue.png) |

### `--opacity=<opacity>` (_double_)

Set _opacity_ on the geometry. Multiplied with the base color texture when present.
Model-specified by default. Usually used with `--blending`.

#### compare

| 1                           | 0.2                           |
| --------------------------- | ----------------------------- |
| ![](./images/opacity_1.png) | ![](./images/opacity_0.2.png) |

### `--roughness=<roughness>` (_double_)

Set the _roughness coefficient_ on the geometry (0.0-1.0). Multiplied with the material texture when present.
Model-specified by default.

#### compare

| 0.2                             | 0.8                             |
| ------------------------------- | ------------------------------- |
| ![](./images/roughness_0.2.png) | ![](./images/roughness_0.8.png) |

### `--metallic=<metallic>` (_double_)

Set the _metallic coefficient_ on the geometry (0.0-1.0). Multiplied with the material texture when present.
Model-specified by default.

#### compare

| 0                            | 1                            |
| ---------------------------- | ---------------------------- |
| ![](./images/metallic_0.png) | ![](./images/metallic_1.png) |

### `--base-ior=<base-ior>` (_double_)

Set the _index of refraction of the base layer_ (1.0-2.5). Model-specified by default.

#### compare

| 1.5 (default)                      | 2.5                            |
| ---------------------------------- | ------------------------------ |
| ![](./images/base_ior_default.png) | ![](./images/base_ior_2.5.png) |

### `--hdri-file=<HDRI file>` (_path_)

Set the _HDRI_ image that can be used as ambient lighting and skybox.
Valid file format are `.hdr`, `.exr`, `.png`, `.jpg`, `.pnm`, `.tiff`, `.bmp`.
If not set, a default is provided.

### `--hdri-ambient` (_bool_, default: `false`)

Light the scene using the _HDRI_ image as ambient lighting.
The environment act as a light source and is reflected on the material.

#### compare

| OFF                                | ON                                |
| ---------------------------------- | --------------------------------- |
| ![](./images/hdri_ambient_off.png) | ![](./images/hdri_ambient_on.png) |

### `--texture-matcap=<texture file>` (_path_)

Set the texture file to control the material capture of the object. All other model options for surfaces are ignored if this is set. Must be in linear color space.
Model-specified by default.

#### compare

| OFF                                       | ON (skin.png)               |
| ----------------------------------------- | --------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/matcap_on.png) |

### `--texture-base-color=<texture file>` (_path_)

Set the texture file to control the color of the object. Please note this will be multiplied with the color and opacity options. Must be in sRGB color space.
Model-specified by default. An empty path can be set to remove the model-specified texture.

#### compare

| OFF                                      | ON                                      |
| ---------------------------------------- | --------------------------------------- |
| ![](./images/texture_base_color_off.png) | ![](./images/texture_base_color_on.png) |

### `--texture-material=<texture file>` (_path_)

Set the texture file to control the occlusion, roughness and metallic values of the object. Please note this will be multiplied with the roughness and metallic options, which have impactful default values. To obtain true results, use `--roughness=1` and `--metallic=1`. Must be in linear color space.
Model-specified by default. An empty path can be set to remove the model-specified texture.

#### compare

| OFF                                    | ON                                    |
| -------------------------------------- | ------------------------------------- |
| ![](./images/texture_material_off.png) | ![](./images/texture_material_on.png) |

### `--texture-emissive=<texture file>` (_path_)

Set the texture file to control the emitted light of the object. Please note this will be multiplied with the emissive factor. Must be in sRGB color space.
Model-specified by default. An empty path can be set to remove the model-specified texture.

#### compare

| OFF                                    | ON                                    |
| -------------------------------------- | ------------------------------------- |
| ![](./images/texture_emissive_off.png) | ![](./images/texture_emissive_on.png) |

### `--emissive-factor=<color>` (_color_)

Set the emissive factor. This value is multiplied with the emissive color when an emissive texture is present.
Model-specified by default.

#### compare

| OFF                                       | Red (2.0, 0.1, 0.1)                   |
| ----------------------------------------- | ------------------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/emissive_factor_red.png) |

### `--texture-normal=<texture file>` (_path_)

Set the texture file to control the normal map of the object. Must be in sRGB color space.
Model-specified by default. An empty path can be set to remove the model-specified texture.

#### compare

| OFF                                  | ON                                  |
| ------------------------------------ | ----------------------------------- |
| ![](./images/texture_normal_off.png) | ![](./images/texture_normal_on.png) |

### `--normal-scale=<factor>` (_double_)

Set the normal scale. This value affects the strength of the normal deviation from the normal texture.
Model-specified by default.

#### compare

| 1                                         | 2                                |
| ----------------------------------------- | -------------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/normal_scale_2.png) |

### `--textures-transform=<transform2d>` (_transform2d_)

Set the 2d transform to use for all textures applied to the model.
Importer may set a default value depending on file type. If a default value exists, the default value is multiplied by the provided transform.

#### compare

| None                                        | Rotated by 10˚                               |
| ------------------------------------------- | -------------------------------------------- |
| ![](./images/texture_transform_default.png) | ![](./images/texture_transform_rotation.png) |

### `--checkerboard` (_bool_, default: `false`)

Override model's texture with a checkerboard texture.

#### compare

| OFF                                       | ON                                |
| ----------------------------------------- | --------------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/checkerboard_on.png) |

### `--unlit` (_bool_)

Override model's lighting configuration.
If the value is true, no shading is applied and the model will be unlit.

#### compare

| OFF                                       | ON                         |
| ----------------------------------------- | -------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/unlit_on.png) |

## Window options

### `--background-color=<color>` (_color_, default: `0.2, 0.2, 0.2`)

Set the window _background color_.
Ignored if `--hdri-skybox` is enabled.

#### compare

| Default                                    | Orange                                    |
| ------------------------------------------ | ----------------------------------------- |
| ![](./images/background_color_default.png) | ![](./images/background_color_orange.png) |

### `--resolution=<width,height>` (_vector\<double\>_, default: `1000, 600`)

Set the _window resolution_.

### `--position=<x,y>` (_vector\<double\>_)

Set the _window position_ (top left corner) , in pixels, starting from the top left of your screens.

### `-z`, `--fps` (_bool_, default: `false`)

Display a rendering _frame per second counter_.

#### compare

| OFF                                       | ON                       |
| ----------------------------------------- | ------------------------ |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/fps_on.png) |

### `-n`, `--filename` (_bool_, default: `false`)

Display the _name of the file_ on top of the window.

#### compare

| OFF                                       | ON                            |
| ----------------------------------------- | ----------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/filename_on.png) |

### `-m`, `--metadata` (_bool_, default: `false`)

Display the _metadata_.

#### compare

| OFF                                       | ON                            |
| ----------------------------------------- | ----------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/metadata_on.png) |

### `--scene-hierarchy` (_bool_, default: `false`)

Display the _scene hierarchy_ as a tree representing the internal structure of the model, with checkboxes allowing to hide or show individual parts.
By default the tree is only expanded enough to show nodes that have meaningful names, but all nodes can be fully expanded manually.

#### compare

| OFF                                       | ON                                   |
| ----------------------------------------- | ------------------------------------ |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/scene_hierarchy_on.png) |

### `--notifications` (_bool_, default: `false`)

Display the notifications at the bottom left of the window.

### `--hdri-filename` (_bool_, default: `false`)

Display the _hdri-filename_.

#### compare

| OFF                                       | ON                                 |
| ----------------------------------------- | ---------------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/hdri_filename_on.png) |

### `--hdri-skybox` (_bool_, default: `false`)

Show the HDRI as a skybox. Overrides `--background-color` and `--no-background`.

#### compare

| OFF                                       | ON                               |
| ----------------------------------------- | -------------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/hdri_skybox_on.png) |

### `-u`, `--blur-background` (_bool_, default: `false`)

Blur background.
Useful with a HDRI skybox.

#### compare

| OFF                             | ON                             |
| ------------------------------- | ------------------------------ |
| ![](./images/hdri_blur_off.png) | ![](./images/hdri_blur_on.png) |

### `--blur-coc` (_double_, default: `20`)

Blur circle of confusion radius.

#### compare

| 20 (default)                       | 50                            |
| ---------------------------------- | ----------------------------- |
| ![](./images/hdri_coc_default.png) | ![](./images/hdri_coc_50.png) |

### `--light-intensity` (_double_, default: `1.0`)

_Adjust the intensity_ of every light in the scene.

#### compare

| 1 (default)                         | 0.5                                   |
| ----------------------------------- | ------------------------------------- |
| ![](./images/light_intensity_1.png) | ![](./images/light_intensity_0.5.png) |

## Scientific visualization options

### `-s`, `--scalar-coloring` (_bool_, default: `false`)

Enable scalar coloring if present in the file. If `--coloring-array` is not set, the first in alphabetical order will be picked if any are available.

#### compare

| OFF                                   | ON                                   |
| ------------------------------------- | ------------------------------------ |
| ![](./images/scalar_coloring_off.png) | ![](./images/scalar_coloring_on.png) |

### `--coloring-array=<array_name>` (_string_)

The coloring array name to use when coloring.
Use `--verbose` to recover the usable array names.

#### compare

| Normals (Z)                             | Height                                  |
| --------------------------------------- | --------------------------------------- |
| ![](./images/coloring_array_normal.png) | ![](./images/coloring_array_height.png) |

### `-y`, `--coloring-component=<comp_index>` (_int_, default: `-1`, implicit: `-2`)

Specify the _component from the scalar_ array to color with.
Use with the scalar option. -1 means _magnitude_. -2 means _direct values_.
When using _direct values_, components are used as L, LA, RGB, RGBA values depending on the number of components.

#### compare

| Normals (X)                            | Normals (Y)                            |
| -------------------------------------- | -------------------------------------- |
| ![](./images/coloring_component_x.png) | ![](./images/coloring_component_y.png) |

### `-c`, `--coloring-by-cells` (_bool_, default: `false`)

Specify that the scalar array is to be found _on the cells_ instead of on the points.
Use with the scalar option.

#### compare

| OFF                                     | ON                                     |
| --------------------------------------- | -------------------------------------- |
| ![](./images/coloring_by_cells_off.png) | ![](./images/coloring_by_cells_on.png) |

### `--coloring-range=<min,max>` (_vector\<double\>_)

Set the _coloring range_. Automatically computed by default.
Use with the scalar option.

#### compare

| Auto                                  | 40,200                           |
| ------------------------------------- | -------------------------------- |
| ![](./images/coloring_range_auto.png) | ![](./images/skull_baseline.png) |

### `-b`, `--coloring-scalar-bar` (_bool_, default: `false`)

Show _scalar bar_ of the coloring by array.
Use with the scalar option.

#### compare

| OFF                              | ON                              |
| -------------------------------- | ------------------------------- |
| ![](./images/skull_baseline.png) | ![](./images/scalar_bar_on.png) |

### `--colormap-file=<name>` (_string_)

Set a _colormap file for the coloring_.
See [color maps](09-COLOR_MAPS.md).
Use with the scalar option.

#### compare

| Default (inferno)                | Viridis                             |
| -------------------------------- | ----------------------------------- |
| ![](./images/skull_baseline.png) | ![](./images/color_map_viridis.png) |

### `--colormap=<colormap>` (_colormap_)

Set a _custom colormap for the coloring_.See [colormap parsing](08-PARSING.md#colormap) for details.
Ignored if `--colormap-file` option is specified.
Use with the scalar option.

### `--colormap-discretization=<colors>` (_int_)

Set the number of distinct colors from [1, N] will be used in the colormap. Any values outside the valid range will result in smooth shading.

#### compare

| Default                                            | 8                                            |
| -------------------------------------------------- | -------------------------------------------- |
| ![](./images/color_map_discretization_default.png) | ![](./images/color_map_discretization_8.png) |

### `-v`, `--volume` (_bool_, default: `false`)

Enable _volume rendering_. It is only functional for 3D image data (VTKXMLVTI, DICOM, NRRD, MetaImage files) and will display nothing with other formats. It forces coloring.

#### compare

| OFF                          | ON                               |
| ---------------------------- | -------------------------------- |
| ![](./images/volume_off.png) | ![](./images/skull_baseline.png) |

### `--volume-opacity-file=<name>` (_string_)

Set an _opacity map file for the coloring_.

#### compare

| Default (linear)                         | Gaussian                                  |
| ---------------------------------------- | ----------------------------------------- |
| ![](./images/volume_opacity_default.png) | ![](./images/volume_opacity_gaussian.png) |

### `--volume-opacity-map=<value, opacity>` (_vector\<double\>_, default: `0.0,0.0,1.0,1.0`)

Set a _custom opacity map for the coloring_.
Only used with volume rendering for now.
Ignored if `--opacity-map-file` option is specified.

### `-i`, `--volume-inverse` (_bool_, default: `false`)

Inverse the linear opacity function used for volume rendering.

#### compare

| OFF                                  | ON                                  |
| ------------------------------------ | ----------------------------------- |
| ![](./images/volume_inverse_off.png) | ![](./images/volume_inverse_on.png) |

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

#### compare

| OFF                              | ON                              |
| -------------------------------- | ------------------------------- |
| ![](./images/raytracing_off.png) | ![](./images/raytracing_on.png) |

### `--raytracing-samples=<samples>` (_int_, default: `5`)

Set the number of _samples per pixel_ when using raytracing.

#### compare

| 2                                      | 6                                      |
| -------------------------------------- | -------------------------------------- |
| ![](./images/raytracing_samples_2.png) | ![](./images/raytracing_samples_6.png) |

### `-d`, `--raytracing-denoise` (_bool_, default: `false`)

_Denoise_ the image when using raytracing.

#### compare

| OFF (2 samples)                          | ON                                      |
| ---------------------------------------- | --------------------------------------- |
| ![](./images/raytracing_denoise_off.png) | ![](./images/raytracing_denoise_on.png) |

## PostFX (OpenGL) options

### `-p`, `--blending` (_string_, default: `none`, implicit: `ddp`)

Enable _translucency blending support_.
This is a technique used to correctly render translucent objects (`ddp`: dual depth peeling for quality, `sort`: for gaussians, `sort_cpu`: for gaussians, `stochastic`: fast).

> [!WARNING]
> `stochastic` is introducing a lot of noise with strong translucency.
> It works better when combined with temporal anti-aliasing (when using `--anti-aliasing=taa` option)
> `sort` is only working for 3D gaussians and requires compute shaders support.
> Alternatively, `sort_cpu` will give the same result and work everywhere but it's much slower.

#### compare

| none                           | ddp                           |
| ------------------------------ | ----------------------------- |
| ![](./images/blending_off.png) | ![](./images/blending_on.png) |

### `-q`, `--ambient-occlusion` (_bool_, default: `false`)

Enable _ambient occlusion_. This is a technique used to improve the depth perception of the object.

#### compare

| OFF                                     | ON                                     |
| --------------------------------------- | -------------------------------------- |
| ![](./images/ambient_occlusion_off.png) | ![](./images/ambient_occlusion_on.png) |

### `-a`, `--anti-aliasing` (_string_, default: `none`, implicit: `fxaa`)

Anti-aliasing method (`fxaa`: fast, `ssaa`: quality, `taa`: balanced, `none`: no anti aliasing)

> [!WARNING]
> `taa` forces rendering of the scene at regular interval and will introduce ghosting artifacts on animated scenes.
> It also doesn't work with offscreen rendering (when using `--output` option)

#### compare

| OFF                                 | ON (SSAA)                          |
| ----------------------------------- | ---------------------------------- |
| ![](./images/anti_aliasing_off.png) | ![](./images/anti_aliasing_on.png) |

### `-t`, `--tone-mapping` (_bool_, default: `false`)

Enable neutral _Tone Mapping_. This technique is used to map colors properly to the monitor colors.

#### compare

| OFF                                       | ON                                |
| ----------------------------------------- | --------------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/tone_mapping_on.png) |

### `--final-shader` (_string_)

Add a final shader to the output image. See the [dedicated documentation](10-FINAL_SHADER.md) for more details.

#### compare

| OFF                                       | ON (grayscale shader)               |
| ----------------------------------------- | ----------------------------------- |
| ![](./images/damaged_helmet_baseline.png) | ![](./images/final_shader_gray.png) |

### `--display-depth` (_bool_, default: `false`)

Display the depth buffer as a grayscale image or with a colormap if `--scalar-coloring` is specified.
Only opaque objects are displayed, the grid and translucent/volumetric objects are ignored.

#### compare

| OFF                                 | ON                                 |
| ----------------------------------- | ---------------------------------- |
| ![](./images/display_depth_off.png) | ![](./images/display_depth_on.png) |

## Testing options

### `--reference=<png file>` (_string_)

Render and compare with the provided _reference image_, for testing purposes. Use with output option to generate new baselines and diff images.

### `--reference-threshold=<threshold>` (_double_, default: `0.04`)

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

## Color space

All options of type _color_ must be expressed in [sRGB](https://en.wikipedia.org/wiki/SRGB) color space. They are converted internally in linear color space for physically correct rendering.

## Options syntax

To turn on/off boolean options, it is possible to write `--option=true` and `--option=false`, eg `--points-sprites=false`.

If an option has an "implicit" value, it means that the options can be used without specifying the value to use the implicit value.

The default correspond to the value without any [configuration file](06-CONFIGURATION_FILE.md), which F3D is usually distributed with.

As documented, the `--option=value` syntax should be preferred. The syntax `--option value` can have unintended effect with positional arguments.

The `-R` short option has a special syntax: `-Rlibf3d.option` but can also be used with `--reset=libf3d.option`

The `-D/--define` option has a special syntax: `-Dlibf3d.option=value` or `--define=libf3d.option=value`.

All options are parsed according to their type, see the [parsing documentation](08-PARSING.md) for more details.

## Piping

F3D supports piping in and out for [most formats](02-SUPPORTED_FORMATS.md), using the `-` char, eg:

```
f3d - --output=- < path/to/file.glb > path/to/img.png
```

or, using [display](https://imagemagick.org/script/display.php#gsc.tab=0):

```
cat path/to/file.glb | f3d - --output=- | display
```

and even, using [build123d](https://github.com/gumyr/build123d):

`script.py`:

```py
import sys

from build123d import Box, Cylinder, export_brep

obj = Box(2, 2, 1) - Cylinder(0.5, 2)
export_brep(obj, sys.stdout.buffer)
```

```
python script.py | f3d - --output=- | display
```

While piping is more common on Linux, F3D supports it perfectly on Windows and MacOS as well.

With versions of VTK < v9.6.20260128, specifying the [reader](02-SUPPORTED_FORMATS.md) to use is required, like this:

```
cat path/to/file.glb --force-reader=GLB | f3d - --output=- | display
```

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
- `{frame}`: frame number when outputting animation frames (see [Animations](05-ANIMATIONS.md))
- `{frame:4}`, `{frame:5}`, ...: zero-padded frame number when outputting animation frames
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
