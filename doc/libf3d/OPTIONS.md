# Options exhaustive list

An option is a specific value stored in different struct in an `options` instance.
They can be accessed directly through the structs, through a string API or through a std::variant API.

The possible option are listed below and are organized by categories and subcategories, here is a non-exhaustive explanation of the categories.

 * `scene` options are related to how the scene is being displayed
 * `render` options are related to the way the render is done
 * `render.effect` options are related to specific techniques used that modify the render
 * `ui` options are related to the screenspace UI element displayed
 * `model` options are related to modifications on the model
 * `interactor` options requires an interactor to be present to have any effect

Please note certain options are taken into account when rendering, others when loading a file.
See the exhaustive list below, but note that this may change in the future.

Please note certain options are `optional` by default, which means they are not set initially,
See the [APIs](#APIs) details below for more info.

## Scene Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
scene.animation.autoplay|bool<br>false<br>load|Automatically start animation.|\-\-animation-autoplay
scene.animation.index|int<br>0<br>load|Select the animation to load.<br>Any negative value means all animations (glTF only).<br>The default scene always has at most one animation.|\-\-animation-index
scene.animation.speed_factor|double<br>1<br>render|Set the animation speed factor to slow, speed up or even invert animation.|\-\-animation-speed-factor
scene.animation.time|double<br>optional<br>load|Set the animation time to load.|\-\-animation-time
scene.animation.frame_rate|double<br>60<br>render|Set the animation frame rate used to play the animation interactively.|\-\-animation-frame-rate
scene.camera.index|int<br>optional<br>load|Select the scene camera to use when available in the file.<br>The default scene always uses automatic camera.|\-\-camera-index
scene.up_direction|string<br>+Y<br>load|Define the Up direction. It impacts the grid, the axis, the HDRI and the camera.|\-\-up
scene.camera.orthographic|bool<br>optional<br>load|Set to true to force orthographic projection. Model specified by default, which is false if not specified.|\-\-camera\-orthographic

## Interactor Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
interactor.axis|bool<br>false<br>render|Show *axes* as a trihedron in the scene.|\-\-axis
interactor.trackball|bool<br>false<br>render|Enable trackball interaction.|\-\-trackball

## Model Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
model.matcap.texture|string<br>optional<br>render|Path to a texture file containing a material capture. All other model options for surfaces are ignored if this is set. Model specified by default.|\-\-texture-matcap
model.color.opacity|double<br>optional<br>render|Set *opacity* on the geometry. Usually used with Depth Peeling option. Multiplied with the `model.color.texture` when present. Model specified by default.|\-\-opacity
model.color.rgb|vector\<double\><br>optional<br>render|Set a *color* on the geometry. Multiplied with the `model.color.texture` when present. Model specified by default.|\-\-color
model.color.texture|string<br>optional<br>render|Path to a texture file that sets the color of the object. Will be multiplied with rgb and opacity. Model specified by default.|\-\-texture-base-color
model.emissive.factor|vector\<double\><br>optional<br>render| Multiply the emissive color when an emissive texture is present. Model specified by default.|\-\-emissive-factor
model.emissive.texture|string<br><br>render|Path to a texture file that sets the emitted light of the object. Multiplied with the `model.emissive.factor`. Model specified by default.|\-\-texture-emissive
model.material.metallic|double<br>optional<br>render|Set the *metallic coefficient* on the geometry (0.0-1.0). Multiplied with the `model.material.texture` when present. Model specified by default.|\-\-metallic
model.material.roughness|double<br>optional<br>render|Set the *roughness coefficient* on the geometry (0.0-1.0). Multiplied with the `model.material.texture` when present. Model specified by default.|\-\-roughness
model.material.texture|string<br>optional<br>render|Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object. Multiplied with the `model.material.roughness` and `model.material.metallic`, set both of them to 1.0 to get a true result. Model specified by default.|\-\-texture-material
model.normal.scale|double<br>optional<br>render|Normal scale affects the strength of the normal deviation from the normal texture. Model specified by default.|\-\-normal-scale
model.normal.texture|string<br>optional<br>render|Path to a texture file that sets the normal map of the object. Model specified by default.|\-\-texture-normal
model.scivis.enable|bool<br>false<br>render|*Color by an array* present in on the data. If `model.scivis.array_name` is not set, the first available array will be used.|\-\-scalar-coloring
model.scivis.cells|bool<br>false<br>render|Color the data with value found *on the cells* instead of points|\-\-cells
model.scivis.colormap|vector\<double\><br>\<inferno\><br>render|Set a *custom colormap for the coloring*.<br>This is a list of colors in the format `val1,red1,green1,blue1,...,valN,redN,greenN,blueN`<br>where all values are in the range (0,1).|\-\-colormap
model.scivis.component|int<br>-1<br>render|Specify the component to color with. -1 means *magnitude*. -2 means *direct values*.|\-\-comp
model.scivis.array_name|string<br><br>render|Select the name of the array to color with.|\-\-coloring-array
model.scivis.range|vector\<double\><br>optional<br>render|Set the *coloring range*. Automatically computed by default.|\-\-range
model.point_sprites.enable|bool<br>false<br>render|Show sphere *points sprites* instead of the geometry.|\-\-point-sprites
model.point_sprites.type|string<br>sphere<br>render|Set the sprites type when showing point sprites (can be `sphere` or `gaussian`).|\-\-point-stripes-type
model.point_sprites.size|double<br>10.0<br>render|Set the *size* of point sprites.|\-\-point-stripes-size
model.volume.enable|bool<br>false<br>render|Enable *volume rendering*. It is only available for 3D image data (vti, dcm, nrrd, mhd files) and will display nothing with other formats. It forces coloring.|\-\-volume
model.volume.inverse|bool<br>false<br>render|Inverse the linear opacity function.|\-\-inverse

## Render Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
render.effect.translucency_support|bool<br>false<br>render|Enable *translucency support*. This is a technique used to correctly render translucent objects, implemented using depth peeling|\-\-translucency-support
render.effect.anti_aliasing|bool<br>false<br>render|Enable *anti-aliasing*. This technique is used to reduce aliasing, implemented using FXAA.|\-\-anti-aliasing
render.effect.ambient_occlusion|bool<br>false<br>render|Enable *ambient occlusion*. This is a technique providing approximate shadows, used to improve the depth perception of the object. Implemented using SSAO|\-\-ambient_occlusion
render.effect.tone_mapping|bool<br>false<br>render|Enable generic filmic *Tone Mapping Pass*. This technique is used to map colors properly to the monitor colors.|\-\-tone-mapping
render.effect.final_shader|string<br>optional<br>render|Add a final shader to the output image|\-\-final-shader. See [user documentation](../user/FINAL_SHADER.md).
render.line_width|double<br>optional<br>render|Set the *width* of lines when showing edges. Model specified by default.|\-\-line-width
render.show_edges|bool<br>optional<br>render|Set to true to show the *cell edges*. Model specified by default.|\-\-edges
render.point_size|double<br>optional<br>render|Set the *size* of points when showing vertices. Model specified by default.|\-\-point-size
render.backface_type|string<br>optional<br>render|Set the Backface type, can be `visible` or `hidden`, model specified by default.|\-\-backface-type
render.grid.enable|bool<br>false<br>render|Show *a grid* aligned with the horizontal (orthogonal to the Up direction) plane.|\-\-grid
render.grid.absolute|bool<br>false<br>render|Position the grid at the *absolute origin* of the model's coordinate system instead of below the model.|\-\-grid
render.grid.unit|double<br>optional<br>render|Set the size of the *unit square* for the grid. Automatically computed by default.|\-\-grid-unit
render.grid.subdivisions|int<br>10<br>render|Set the number of subdivisions for the grid.|\-\-grid-subdivisions
render.grid.color|int<br>(0, 0, 0)<br>render|Set the color of grid lines.|\-\-grid-color
render.raytracing.enable|bool<br>false<br>render|Enable *raytracing*. Requires the raytracing module to be enabled.|\-\-raytracing
render.raytracing.samples|int<br>5<br>render|The number of *samples per pixel*.|\-\-samples
render.raytracing.denoise|bool<br>false<br>render|*Denoise* the raytracing rendering.|\-\-denoise
render.hdri.file|string<br>optional<br>render|Set the *HDRI* image that can be used for ambient lighting and skybox.<br>Valid file format are hdr, exr, png, jpg, pnm, tiff, bmp.<br>If not set, a default is provided.|\-\-hdri-file
render.hdri.ambient|bool<br>false<br>render|Light the scene using the *HDRI* image as ambient lighting<br>The environment act as a light source and is reflected on the material.|\-\-hdri-ambient
render.background.color|vector\<double\><br>0.2,0.2,0.2<br>render|Set the window *background color*.<br>Ignored if a *hdri* skybox is used.|\-\-bg-color
render.background.skybox|bool<br>false<br>render|Show the *HDRI* image as a skybox<br>Overrides the the background color if any|\-\-hdri-skybox
render.background.blur|bool<br>false<br>render|Blur background, useful with a skybox.|\-\-blur-background
render.background.blur.coc|double<br>20.0<br>render|Blur background circle of confusion radius.|\-\-blur-coc
render.light.intensity|double<br>1.0<br>render|Adjust the intensity of every light in the scene.|\-\-light-intensity

## UI Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
ui.scalar_bar|bool<br>false<br>render|Show *scalar bar* of the coloring by data array.|\-\-bar
ui.cheatsheet|bool<br>false<br>render|Show a interactor cheatsheet
ui.filename|bool<br>false<br>render|Display the *filename info content* on top of the window.|\-\-filename
ui.filename_info|string<br>-<br>render|Content of *filename info* to display.
ui.font_file|string<br>optional<br>render|Use the provided FreeType compatible font file to display text.<br>Can be useful to display non-ASCII filenames.|\-\-font-file
ui.fps|bool<br>false<br>render|Display a *frame per second counter*.|\-\-fps
ui.loader_progress|bool<br>false<br>load|Show a *progress bar* when loading the file.|\-\-progress
ui.animation_progress|bool<br>false<br>load|Show a *progress bar* when playing the animation.|\-\-animation-progress
ui.metadata|bool<br>false<br>render|Display the *metadata*.|\-\-metadata
ui.dropzone|bool<br>false<br>render|Show a drop zone.
ui.dropzone_info|string<br>-<br>render|Content of the drop zone text to display.

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
    std::cout << "Line Width: " << opt.render.line_width.value() << std::endl;
  }
  else
  {
    std::cout << "Line Width: unset" << std::endl;
  }
```

It's even more true with the few optional boolean options as std::optional has an implicit boolean cast operator.

## String API

The most generic and flexible API, as it rely on parsing and string generation.
The documentation about option parsing is upcoming.

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
    std::cout << userProvidedName << ": " << opt.getAsString(userProvidedName) << std::endl;
  }
  catch (const f3d::options::inexistent_exception&)
  {
    std::cout << userProvidedName << " does not exist." << std::endl;
  }
  catch (const f3d::options::no_value_exception&)
  {
    std::cout << userProvidedName << " is not set." << std::endl;
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
