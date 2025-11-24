# Options

An option is a specific value stored in different struct in an `options` instance.
They can be accessed directly through the structs, through a string API or through a std::variant API.

The possible option are listed below and are organized by categories and subcategories, here is a non-exhaustive explanation of the categories.

- `scene` options are related to how the scene is being displayed
- `render` options are related to the way the render is done
- `render.effect` options are related to specific techniques used that modify the render
- `ui` options are related to the screenspace UI element displayed
- `model` options are related to modifications on the model
- `interactor` options requires an interactor to be present to have any effect

Please note certain options are taken into account when rendering, others when loading a file.
See the exhaustive list below.

Please note certain options are `optional` by default, which means they are not set initially,
See the [APIs](#struct-api) details below for more info.

## Scene Options

### `scene.animation.autoplay` (_bool_, default: `false`, **on load**)

Automatically start animation.

CLI: `--animation-autoplay`.

### `scene.animation.indices` (_vector\<int\>_, default: `0`, **on load**)

Select the animations to load.
Any negative value means all animations.
The default scene always has at most one animation.

CLI: `--animation-indices`.

### `scene.animation.speed_factor` (_ratio_, default: `1`)

Set the animation speed factor to slow, speed up or even invert animation.

CLI: `--animation-speed-factor`.

### `scene.animation.time` (_double_, optional, **on load**)

Set the animation time to load.

CLI: `--animation-time`.

### `scene.camera.index` (_int_, optional, **on load**)

Select the scene camera to use when available in the file.
The default scene always uses automatic camera.

CLI: `--camera-index`.

### `scene.up_direction` (_direction_, default: `+Y`, **on load**)

Define the Up direction. It impacts the grid, the axis, the HDRI and the camera.
CLI: `--up`.

### `scene.force_reader` (_string_, optional, **on load**)

Force a specific reader to be used, disregarding the file extension. See [user documentation](../user/02-SUPPORTED_FORMATS.md).

CLI: `--force-reader`.

### `scene.camera.orthographic` (_bool_, optional)

Set to true to force orthographic projection. Model-specified by default, which is false if not specified.

CLI: `--camera-orthographic`.

## Interactor Options

### `interactor.trackball` (_bool_, default: `false`)

Enable trackball interaction.

CLI: `--trackball`.

### `interactor.invert_zoom` (_bool_, default: `false`)

Invert the zoom direction.

CLI: `--invert-zoom`.

## Model Options

### `model.matcap.texture` (_path_, optional)

Path to a texture file containing a material capture. All other model options for surfaces are ignored if this is set. Model-specified by default.

CLI: `--texture-matcap`.

### `model.color.opacity` (_double_, optional)

Set _opacity_ on the geometry. Usually used with Depth Peeling option. Multiplied with the `model.color.texture` when present. Model-specified by default.

CLI: `--opacity`.

### `model.color.rgb` (_color_, optional)

Set a _color_ on the geometry. Multiplied with the `model.color.texture` when present. Model-specified by default.

CLI: `--color`.

### `model.color.texture` (_path_, optional)

Path to a texture file that sets the color of the object. Will be multiplied with rgb and opacity. Model-specified by default.

CLI: `--texture-base-color`.

### `model.emissive.factor` (_color_, optional)

Multiply the emissive color when an emissive texture is present. Model-specified by default.

CLI: `--emissive-factor`.

### `model.emissive.texture` (_path_, optional)

Path to a texture file that sets the emitted light of the object. Multiplied with the `model.emissive.factor`. Model-specified by default.

CLI: `--texture-emissive`.

### `model.material.metallic` (_double_, optional)

Set the _metallic coefficient_ on the geometry (between `0.0` and `1.0`). Multiplied with the `model.material.texture` when present. Model-specified by default.

CLI: `--metallic`.

### `model.material.roughness` (_double_, optional)

Set the _roughness coefficient_ on the geometry (between `0.0` and `1.0`). Multiplied with the `model.material.texture` when present. Model-specified by default.

CLI: `--roughness`.

### `model.material.base_ior` (_double_, optional)

Set the _index of refraction of the base layer_ (between `1.0` and `2.5`). Model-specified by default.

CLI: `--base-ior`.

### `model.material.texture` (_path_, optional)

Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object. Multiplied with the `model.material.roughness` and `model.material.metallic`, set both of them to `1.0` to get a true result. Model-specified by default.

CLI: `--texture-material`.

### `model.normal.scale` (_double_, optional)

Normal scale affects the strength of the normal deviation from the normal texture. Model-specified by default.

CLI: `--normal-scale`.

### `model.normal.texture` (_path_, optional)

Path to a texture file that sets the normal map of the object. Model-specified by default.

CLI: `--texture-normal`.

### `model.scivis.enable` (_bool_, default: `false`)

_Color by an array_ present in on the data. If `model.scivis.array_name` is not set, the first available array will be used.

CLI: `--scalar-coloring`.

### `model.scivis.cells` (_bool_, default: `false`)

Color the data with value found _on the cells_ instead of points

CLI: `--cells`.

### `model.scivis.discretization` (_int_, optional)

Set how many distinct colors will be used from the colormap. Should be `>=1`.

CLI: `--colormap-discretization`.

### `model.scivis.colormap` (_colormap_, default: `<inferno>`)

Set a _custom colormap for the coloring_.See [colormap parsing](../user/08-PARSING.md#colormap) for details.

CLI: `--colormap`.

### `model.scivis.component` (_int_, default: `-1`)

Specify the component to color with. `-1` means _magnitude_. `-2` means _direct values_.

CLI: `--comp`.

### `model.scivis.array_name` (_string_, optional)

Select the name of the array to color with.

CLI: `--coloring-array`.

### `model.scivis.range` (_vector\<double\>_, optional)

Set the _coloring range_. Automatically computed by default.

CLI: `--range`.

### `model.point_sprites.enable` (_bool_, default: `false`)

Show sphere _points sprites_ instead of the geometry.

CLI: `--point-sprites`.

### `model.point_sprites.type` (_string_, default: `sphere`)

Set the sprites type when showing point sprites (can be `sphere` or `gaussian`).

CLI: `--point-sprites-type`.

### `model.point_sprites.size` (_double_, default: `10.0`)

Set the _size_ of point sprites.

CLI: `--point-sprites-size`.

### `model.volume.enable` (_bool_, default: `false`)

Enable _volume rendering_. It is only available for 3D image data and will display nothing with incompatible data. It forces coloring.

CLI: `--volume`.

### `model.volume.inverse` (_bool_, default: `false`)

Inverse the linear opacity function.

CLI: `--inverse`.

### `model.textures_transform` (_transform2d_, optional)

Transform applied to textures on the model. If a default transform is set by the importer, the default value will be multiplied by this transform.

CLI: `--textures-transform`.

## Render Options

### `render.effect.blending.enable` (_bool_, default: `false`)

Enable _blending support_. This is a technique used to correctly render translucent objects.

CLI: `--blending`.

### `render.effect.blending.mode` (_string_, default: `ddp`)

Set the blending technique. Valid options are: `ddp` (dual depth peeling, quality), `sort` (only for gaussians), `stochastic` (fast)

CLI: `--blending`.

### `render.effect.antialiasing.enable` (_string_, default: `false`)

Enable _anti-aliasing_. This technique is used to reduce aliasing.

CLI: `--anti-aliasing`.

### `render.effect.antialiasing.mode` (_string_, default: `fxaa`)

Set the anti-aliasing technique. Valid options are: `fxaa` (fast), `ssaa` (quality), `taa` (balanced)

CLI: `--anti-aliasing`.

### `render.effect.ambient_occlusion` (_bool_, default: `false`)

Enable _ambient occlusion_. This is a technique providing approximate shadows, used to improve the depth perception of the object. Implemented using SSAO

CLI: `--ambient-occlusion`.

### `render.effect.tone_mapping` (_bool_, default: `false`)

Enable generic filmic _Tone Mapping Pass_. This technique is used to map colors properly to the monitor colors.

CLI: `--tone-mapping`.

### `render.effect.final_shader` (_string_, optional)

Add a final shader to the output image. See [user documentation](../user/10-FINAL_SHADER.md).

CLI: `--final-shader`

### `render.line_width` (_double_, optional)

Set the _width_ of lines when showing edges. Model-specified by default.

CLI: `--line-width`.

### `render.show_edges` (_bool_, optional)

Set to true to show the _cell edges_. Model-specified by default.

CLI: `--edges`.

### `render.point_size` (_double_, optional)

Set the _size_ of points when showing vertices. Model-specified by default.

CLI: `--point-size`.

### `render.backface_type` (_string_, optional, **on load**)

Set the Backface type, can be `visible` or `hidden`, Model-specified by default.

CLI: `--backface-type`.

### `render.grid.enable` (_bool_, default: `false`)

Show _a grid_ aligned with the horizontal (orthogonal to the Up direction) plane.

CLI: `--grid`.

### `render.grid.absolute` (_bool_, default: `false`)

Position the grid at the _absolute origin_ of the model's coordinate system instead of below the model.

CLI: `--grid-absolute`.

### `render.grid.unit` (_double_, optional)

Set the size of the _unit square_ for the grid. Automatically computed by default.

CLI: `--grid-unit`.

### `render.grid.subdivisions` (_int_, default: `10`)

Set the number of subdivisions for the grid.

CLI: `--grid-subdivisions`.

### `render.grid.color` (_color_, default: `0, 0, 0`)

Set the color of grid lines.

CLI: `--grid-color`.

### `render.raytracing.enable` (_bool_, default: `false`)

Enable _raytracing_. Requires the raytracing module to be enabled.

CLI: `--raytracing`.

### `render.raytracing.samples` (_int_, default: `5`)

The number of _samples per pixel_.

CLI: `--samples`.

### `render.raytracing.denoise` (_bool_, default: `false`)

_Denoise_ the raytracing rendering.

CLI: `--denoise`.

### `render.hdri.file` (_path_, optional)

Set the _HDRI_ image that can be used for ambient lighting and skybox.
Valid file format are `.hdr`, `.exr`, `.png`, `.jpg`, `.pnm`, `.tiff`, `.bmp`.
If not set, a default is provided.

CLI: `--hdri-file`.

### `render.hdri.ambient` (_bool_, default: `false`)

Light the scene using the _HDRI_ image as ambient lighting
The environment act as a light source and is reflected on the material.

CLI: `--hdri-ambient`.

### `render.background.color` (_color_, default: `0.2,0.2,0.2`)

Set the window _background color_.
Ignored if a _hdri_ skybox is used.

CLI: `--background-color`.

### `render.background.skybox` (_bool_, default: `false`)

Show the _HDRI_ image as a skybox
Overrides the the background color if any

CLI: `--hdri-skybox`.

### `render.background.blur.enable` (_bool_, default: `false`)

Blur background, useful with a skybox.

CLI: `--blur-background`.

### `render.background.blur.coc` (_double_, default: `20.0`)

Blur background circle of confusion radius.

CLI: `--blur-coc`.

### `render.light.intensity` (_double_, default: `1.0`)

Adjust the intensity of every light in the scene.

CLI: `--light-intensity`.

### `render.armature.enable` (_bool_, default: `false`)

Display armatures if present in the scene (glTF only).

CLI: `--armature`.

## UI Options

### `ui.axis` (_bool_, default: `false`)

Show _axes_ as a trihedron in the scene. Requires an interactor.

CLI: `--axis`.

### `ui.scalar_bar` (_bool_, default: `false`)

Show _scalar bar_ of the coloring by data array.

CLI: `--bar`.

### `ui.cheatsheet` (_bool_, default: `false`)

Show a interaction cheatsheet

### `ui.console` (_bool_, default: `false`)

Show the console

### `ui.minimal_console` (_bool_, default: `false`)

Show the minimal console. Useful to execute a quick command and exit.

### `ui.filename` (_bool_, default: `false`)

Display the _filename info content_ on top of the window.

### `ui.filename_info` (_string_, default: `""`)

Content of _filename info_ to display.

### `ui.hdri_filename` (_bool_, default: `false`)

Display the _HDRI filename info content_ on top of the window.

CLI: `--hdri-filename`.

### `ui.font_file` (_path_, optional)

Use the provided FreeType compatible font file to display text.
Can be useful to display non-ASCII filenames.

CLI: `--font-file`.

### `ui.scale` (_ratio_, default: `1.0`)

Scale fonts. Useful for HiDPI displays.

CLI: `--font-scale`.

### `ui.fps` (_bool_, default: `false`)

Display a _frame per second counter_.

CLI: `--fps`.

### `ui.loader_progress` (_bool_, default: `false`, **on load**)

Show a _progress bar_ when loading the file.

### `ui.loader_progress_color` (_color_, optional, **on load**)

Set loader progress bar color or use default one #FFFFFF if not set.

CLI: `--progress`.

### `ui.animation_progress` (_bool_, default: `false`, **on load**)

Show a _progress bar_ when playing the animation.

CLI: `--animation-progress`.

### `ui.animation_progress_color` (_color_, optional, **on load**)

Set animation progress bar color or use default one #788BFF if not set.

### `ui.metadata` (_bool_, default: `false`)

Display the _metadata_.

CLI: `--metadata`.

### `ui.drop_zone.enable` (_bool_, default: `false`)

Show a drop zone. Rendering is disabled when the viewport is smaller than 10×10 pixels.

### `ui.drop_zone.custom_binds` (_string_, default: `""`)

Space separated custom key bindings to display in the drop zone.

### `ui.drop_zone.show_logo` (_bool_, default: `false`)

Display the logo. Needs ui.drop_zone.enable to be true.

### `ui.backdrop.opacity` (_double_, default: `0.9`)

Set the opacity of the backdrop behind the UI elements. Value is between 0.0 (fully transparent) and 1.0 (fully opaque).

CLI: `--backdrop-opacity`.

# APIs

There are three APIs to access the options

## Struct API

The most straightforward and easy to use API, just access it through the structs available in the options instance, eg:

```cpp
  f3d::engine eng = f3d::engine::create();
  f3d::options& opt = eng.getOptions();
  opt.render.show_edges = true;
  opt.render.grid.enable = true;
  opt.ui.metadata = true;
  opt.model.material.roughness = 0.6;
```

Please note that when accessing optional options, special care must be used, eg:

```cpp
  f3d::engine eng = f3d::engine::create();
  f3d::options& opt = eng.getOptions();
  if (opt.render.line_width.has_value())
  {
    std::cout << "Line Width: " << opt.render.line_width.value() << "\n";
  }
  else
  {
    std::cout << "Line Width: unset\n";
  }
```

It's even more true with the few optional boolean options as std::optional has an implicit boolean cast operator.

## String API

The most generic and flexible API, as it rely on parsing and string generation.
See the [parsing documentation](../user/08-PARSING.md) for more details.

```cpp
  f3d::engine eng = f3d::engine::create();
  f3d::options& opt = eng.getOptions();
  opt.setAsString("render.show_edges", "true");
  opt.setAsString("render.grid.enable", "true");
  opt.setAsString("ui.metadata", "true");
  opt.setAsString("model.material.roughness", "0.6");
```

When using this API make sure to catch exceptions has needed, eg:

```cpp
  f3d::engine eng = f3d::engine::create();
  f3d::options& opt = eng.getOptions();

  try
  {
    std::cout << userProvidedName << ": " << opt.getAsString(userProvidedName) << "\n";
  }
  catch (const f3d::options::inexistent_exception&)
  {
    std::cout << userProvidedName << " does not exist.\n";
  }
  catch (const f3d::options::no_value_exception&)
  {
    std::cout << userProvidedName << " is not set.\n";
  }
```

## Variant API

An API that is similar to the F3D 2.0 options API thanks to std::variant.

```cpp
  f3d::engine eng = f3d::engine::create();
  f3d::options& opt = eng.getOptions();
  opt.set("render.show_edges", true);
  opt.set("render.grid.enable", true);
  opt.set("ui.metadata", true);
  opt.set("model.material.roughness", 0.6);
```

When using this API make sure to catch exception shown above with the string API.
