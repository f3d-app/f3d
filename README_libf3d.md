# libf3d - A library to render 3D meshes

By Michael Migliore and Mathieu Westphal.

libf3d is a BSD-licensed C++ library to open and render 3D meshes. It is of course used by F3D.
libf3d API is simple and easy to learn. Python bindings are provided through pybind11.

# Getting Started

Rendering a file and starting the interaction is very easy:

```
// Create a f3d::engine
f3d::engine eng();

// Recover the file loader
f3d::loader& load = eng.getLoader();

// Add a file to the loader
load.addFile("path/to/file.ext");

// Load the file
load.loadFile();

// Recover the window
f3d::window& win = eng.getWindow();

// Render the file in the window
win.render();

// Recover the interactor
f3d::interactor& inter = eng.getInteractor();

// Start interacting
inter.start();
```

Changing some options can be done this way:

```
f3d::engine eng();

// Recover the options
f3d::options& options = eng.getOptions();

// Set the option to the wanted value
options.set("options.render.ssao", true);

// Standard libf3d usage
f3d::loader& load = eng.getLoader();
load.addFile("path/to/file.ext");
load.loadFile();
f3d::window& win = eng.getWindow();
f3d::interactor& inter = eng.getInteractor();
inter.start();
```
Most options are dynamic, some are only taken into account when loading a file. See the ##options## documentation.

For more advanced usage, please take a look at the testing directory.

# Engine class

The engine class is the main class that needs to be instantiated. All other classes instance are provided by the engine using getters, `getLoader`, `getWindow`, `getInteractor`, `getOptions`.

The engine constructor let you choose the type of window in its constructor, `NONE`, `NATIVE`, `NATIVE_OFFSCREEN`, `EXTERNAL`. Default is `NATIVE`. See ##Window class## documentation for more info. Please note that the engine will not provide a interactor with `NONE` and `EXTERNAL`.

# Loader class

The loader class is responsible to read and load the file from the disk. It supports reading multiple files and even folders.

# Window class

The window class is responsible for rendering the meshes. It supports multiple modes.

* NONE
A window that will not render anything, very practical when only trying to recover meta-information about the data.

* NATIVE
Default mode where a window is shown onscreen using native graphical capabilities.

* NATIVE_OFFSCREEN
Use native graphical capabilities for rendering, but unto an offscreen window, which will not appear on screen, practical when generating screenshots.

* EXTERNAL
A window where the OpenGL context is not created but assumed to have been created externally. To be used with other frameworks like Qt or GLFW.

Window let you `render`, `renderToImage` and control other parameters of the window, like icon or windowName.

# Interactor class

When provided by the engine, the interactor class lets you choose how to interact with the data.

It contains the animation API to start and stop it.
Interactor also lets you set your interaction callbacks in order to modify how the interaction with the data is done.
Of course, you can use `start` and `stop` to control it.
It also lets you define you own callbacks when needed.

# Camera class

Provided by the window, this class let you control the camera. There are essentially three API.
 1. A Position/FocalPoint/ViewUp API to put the camera where you want it.
 2. A camera movement API to move the camera around
 3. A ViewMatrix API to work with matrices directly.

Please note that the API 1. and 2. can be used together and always orthogonalize the view up after each call.
API 3. is not intended to be used with other API as it can result to unexpected cameras.

# Image class

A generic image class that can either be created from a window, from an image filepath or even from a data buffer. It supports comparison making it very practical in testing context.

# Log class

A class to control logging in the libf3d. Simple using the different dedicated methods (`print`, `debug`, `info`, `warn`, `error`) and `setVerboseLevel`, you can easily control what to display. Please note that, on windows, a dedicated output window may be created.

# Option class

This class lets you control the behavior of the libf3d. An option is basically a string used as a key associated with a value.
Options are organized by categories and subcategories, here is a non-exhaustive explanation of the categories.

 * `scene` options are related to how the scene is being displayed
 * `render` options are related to the way the render is done
 * `render.effect` options are related to specific techniques used that modify the render
 * `ui` options are related to the screenspace UI element displayed
 * `model` options are related to modifications on the model, they are only meaningful when using the default scene
 * `interactor` options requires an interactor to be present to have any effect.

Please note certain options are taken into account when rendering, others when loading a file.
See the exhaustive list below, but note that this may change in the future.

## Scene Options
Options|Default|Type|Description|F3D option|Trigger
------|------|------|------|------|------
scene.animation.index|0|int|Select the animation to load.<br>Any negative value means all animations.<br>The default scene always has at most one animation.<br>By default, the first animation is enabled.|--animation-index|load
scene.camera.index|-1|int|Select the scene camera to use when available in the file.<br>Any negative value means automatic camera.<br>The default scene always uses automatic camera.|--camera-index|load
scene.geometry-only|false|bool|For certain **full scene** file formats (gltf/glb and obj),<br>reads *only the geometry* from the file and use default scene construction instead.|--geometry-only|load
scene.up-direction|+Y|string|Define the Up direction|--up|load
scene.grid|false|bool|Show *a grid* aligned with the XZ plane.|--grid|render
scene.background.blur|false|bool|Blur background.<br>This only makes sense when using a HDRI.|--blur-background|render
scene.background.color|0.2,0.2,0.2|vector<double>|Set the window *background color*.<br>Ignored if *hdri* is set.|--bg-color|render
scene.background.hdri||string|Set the *HDRI* image used to create the environment.<br>The environment act as a light source and is reflected on the material.<br>Valid file format are hdr, png, jpg, pnm, tiff, bmp. Override the color.|--hdri|render

interactor.axis|false|bool|Show *axes* as a trihedron in the scene.|--axis|render
interactor.trackball|false|bool|Enable trackball interaction.|--trackball|render

## Model Options
Options|Default|Type|Description|F3D option|Trigger
------|------|------|------|------|------
model.color.opacity|1.0|double|Set *opacity* on the geometry. Usually used with Depth Peeling option. Multiplied with the `model.color.texture` when present.|--opacity|load
model.color.rgb|1.0,1.0,1.0|vector<double>|Set a *color* on the geometry. Multiplied with the `model.color.texture` when present.|--color|load
model.color.texture||string|Path to a texture file that sets the color of the object. Will be mulitplied with rgb and opacity.|--texture-base-color|load
model.emissive.factor|1.0,1.0,1.0|vector<double>| Multiply the emissive color when an emissive texture is present.|--emissive-factor|load
model.emissive.texture||string|Path to a texture file that sets the emitted light of the object. Multiplied with the `model.emissive.factor`.|--texture-emissive|load
model.material.metallic|0.0|double|Set the *metallic coefficient* on the geometry (0.0-1.0). Multiplied with the `model.material.texture` when present.|--metallic|load
model.material.roughness|0.3|double|Set the *roughness coefficient* on the geometry (0.0-1.0). Multiplied with the `model.material.texture` when present.|--roughness|load
model.material.texture||string|Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object. Multiplied with the `model.material.roughness` and `model.material.metallic`, set both of them to 1.0 to get a true result.|--texture-material|load
model.normal.scale|1.0|double|Normal scale affects the strength of the normal deviation from the normal texture.|--normal-scale|load
model.normal.texture||string|Path to a texture file that sets the normal map of the object.|--texrture-normal|load
model.scivis.cells|false|bool|Color the data with value found *on the cells* instead of points|--cells|render
model.scivis.colormap|<inferno>|vector<double>|Set a *custom colormap for the coloring*.<br>This is a list of colors in the format `val1,red1,green1,blue1,...,valN,redN,greenN,blueN`<br>where all values are in the range (0,1).|--colormap|render
model.scivis.component|-1|int|Specify the component to color with. -1 means *magnitude*. -2 means *direct values*.|--comp|render
model.scivis.array-name|<reserved>|string|*Color by a specific data array* present in on the data. Set to <empty> to let libf3d find the first available array.|--scalars|render
model.scivis.range||vector<double>|Set a *custom range for the coloring*.|--range|render

## Render Options
Options|Default|Type|Description|F3D option|Trigger
------|------|------|------|------|------
render.effect.depth-peeling|false|bool|Enable *depth peeling*. This is a technique used to correctly render translucent objects.|--depth-peeling|render
render.effect.fxaa|false|bool|Enable *Fast Approximate Anti-Aliasing*. This technique is used to reduce aliasing.|--fxaa|render
render.effect.ssao|false|bool|Enable *Screen-Space Ambient Occlusion*. This is a technique used to improve the depth perception of the object.|--ssao|render
render.effect.tone-mapping|false|bool|Enable generic filmic *Tone Mapping Pass*. This technique is used to map colors properly to the monitor colors.|--tone-mapping|render
render.line-width|1.0|double|Set the *width* of lines when showing edges.|--line-width|render
render.show-edges|false|bool|Show the *cell edges*|--edges|render
render.point-size|10.0|double|Set the *size* of points when showing vertices and point sprites.|--point-size|render
render.point-sprites.enabled|false|bool|Show sphere *points sprites* instead of the geometry.|--point-sprites|render
render.volume.enabled|false|bool|Enable *volume rendering*. It is only available for 3D image data (vti, dcm, nrrd, mhd files) and will display nothing with other default scene formats.|--volume|render
render.volume.inverse|false|bool|Inverse the linear opacity function.|--inverse|render
render.raytracing.denoise|false|bool|*Denoise* the raytracing rendering.|--denoise|render
render.raytracing.enable|false|bool|Enable *raytracing*. Requires the raytracing module to be enabled.|--raytracing|render
render.raytracing.samples|5|int|The number of *samples per pixel*.|--samples|render

## UI Options
Options|Default|Type|Description|F3D option|Trigger
------|------|------|------|------|------
ui.bar|false|bool|Show *scalar bar* of the coloring by data array.|--bar|render
ui.cheatsheet|false|bool|Show a interactor cheatsheet|render
ui.filename|false|bool|Display the *name of the file*.|--filename|render
ui.font-file||string|Use the provided FreeType compatible font file to display text.<br>Can be useful to display non-ASCII filenames.|--font-file|render
ui.fps|false|bool|Display a *frame per second counter*.|--fps|render
ui.loader-progress|false|bool|Show a *progress bar* when loading the file.|--progress|load
ui.metadata|false|bool|Display the *metadata*.|--metadata|render

## Window Options TODO remove
Options|Default|Type|Description|F3D option|Trigger
------|------|------|------|------|------
window.fullscreen|false|bool|Display in fullscreen.|--fullscreen|render

# Python Bindings

If the python bindings are generated, the libf3d can be used directly from python.
Make sure to set `PYTHONPATH` to path where the python module is built.
Here is an example showing how to use libf3d python bindings:

```python
import f3d

engine = f3d.engine(f3d.engine.CREATE_WINDOW | f3d.engine.CREATE_INTERACTOR)

engine.getOptions().set("scalars", "Normals")
engine.getOptions().set("component", 0)
engine.getOptions().set("bar", True)
engine.getOptions().set("grid", True)

engine.getLoader().addFile("f3d/testing/data/dragon.vtu")
engine.getLoader().loadFile(f3d.loader.LoadFileEnum.LOAD_CURRENT)

engine.getInteractor().start()
```
