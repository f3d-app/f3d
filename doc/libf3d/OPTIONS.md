# Options exhaustive list

An option is a string used as a key associated with a value, which are stored in an `options` instance.
The possible option are listed below and are organized by categories and subcategories, here is a non-exhaustive explanation of the categories.

 * `scene` options are related to how the scene is being displayed
 * `render` options are related to the way the render is done
 * `render.effect` options are related to specific techniques used that modify the render
 * `ui` options are related to the screenspace UI element displayed
 * `model` options are related to modifications on the model, they are only meaningful when using the default scene
 * `interactor` options requires an interactor to be present to have any effect.

Please note certain options are taken into account when rendering, others when loading a file.
See the exhaustive list below, but note that this may change in the future.

## Scene Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
scene.animation.index|int<br>0<br>load|Select the animation to load.<br>Any negative value means all animations.<br>The default scene always has at most one animation.|\-\-animation-index
scene.camera.index|int<br>-1<br>load|Select the scene camera to use when available in the file.<br>Any negative value means automatic camera.<br>The default scene always uses automatic camera.|\-\-camera-index
scene.geometry-only|bool<br>false<br>load|For certain **full scene** file formats (gltf/glb and obj),<br>reads *only the geometry* from the file and use default scene construction instead.|\-\-geometry-only
scene.up-direction|string<br>+Y<br>load|Define the Up direction|\-\-up
scene.grid|bool<br>false<br>render|Show *a grid* aligned with the XZ plane.|\-\-grid
scene.background.blur|bool<br>false<br>render|Blur background when using a HDRI.|\-\-blur-background
scene.background.color|vector\<double\><br>0.2,0.2,0.2<br>render|Set the window *background color*.<br>Ignored if *hdri* is set.|\-\-bg-color
scene.background.hdri|string<br>-<br>render|Set the *HDRI* image used to create the environment.<br>The environment act as a light source and is reflected on the material.<br>Valid file format are hdr, png, jpg, pnm, tiff, bmp. Override the color.|\-\-hdri

## Interactor Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
interactor.axis|bool<br>false<br>render|Show *axes* as a trihedron in the scene.|\-\-axis
interactor.trackball|bool<br>false<br>render|Enable trackball interaction.|\-\-trackball

## Model Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
model.color.opacity|double<br>1.0<br>load|Set *opacity* on the geometry. Usually used with Depth Peeling option. Multiplied with the `model.color.texture` when present.|\-\-opacity
model.color.rgb|vector\<double\><br>1.0,1.0,1.0<br>load|Set a *color* on the geometry. Multiplied with the `model.color.texture` when present.|\-\-color
model.color.texture|string<br>-<br>load|Path to a texture file that sets the color of the object. Will be multiplied with rgb and opacity.|\-\-texture-base-color
model.emissive.factor|vector\<double\><br>1.0,1.0,1.0<br>load| Multiply the emissive color when an emissive texture is present.|\-\-emissive-factor
model.emissive.texture|string<br>-<br>load|Path to a texture file that sets the emitted light of the object. Multiplied with the `model.emissive.factor`.|\-\-texture-emissive
model.material.metallic|double<br>0.0<br>load|Set the *metallic coefficient* on the geometry (0.0-1.0). Multiplied with the `model.material.texture` when present.|\-\-metallic
model.material.roughness|double<br>0.3<br>load|Set the *roughness coefficient* on the geometry (0.0-1.0). Multiplied with the `model.material.texture` when present.|\-\-roughness
model.material.texture|string<br>-<br>load|Path to a texture file that sets the Occlusion, Roughness and Metallic values of the object. Multiplied with the `model.material.roughness` and `model.material.metallic`, set both of them to 1.0 to get a true result.|\-\-texture-material
model.normal.scale|double<br>1.0<br>load|Normal scale affects the strength of the normal deviation from the normal texture.|\-\-normal-scale
model.normal.texture|string<br>-<br>load|Path to a texture file that sets the normal map of the object.|\-\-texrture-normal
model.scivis.cells|bool<br>false<br>render|Color the data with value found *on the cells* instead of points|\-\-cells
model.scivis.colormap|vector\<double\><br>\<inferno\><br>render|Set a *custom colormap for the coloring*.<br>This is a list of colors in the format `val1,red1,green1,blue1,...,valN,redN,greenN,blueN`<br>where all values are in the range (0,1).|\-\-colormap
model.scivis.component|int<br>-1<br>render|Specify the component to color with. -1 means *magnitude*. -2 means *direct values*.|\-\-comp
model.scivis.array-name|string<br>\<reserved\><br>render|*Color by a specific data array* present in on the data. Set to <empty> to let libf3d find the first available array.|\-\-scalars
model.scivis.range|vector\<double\><br>-<br>render|Set a *custom range for the coloring*.|\-\-range

## Render Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
render.effect.depth-peeling|bool<br>false<br>render|Enable *depth peeling*. This is a technique used to correctly render translucent objects.|\-\-depth-peeling
render.effect.fxaa|bool<br>false<br>render|Enable *Fast Approximate Anti-Aliasing*. This technique is used to reduce aliasing.|\-\-fxaa
render.effect.ssao|bool<br>false<br>render|Enable *Screen-Space Ambient Occlusion*. This is a technique used to improve the depth perception of the object.|\-\-ssao
render.effect.tone-mapping|bool<br>false<br>render|Enable generic filmic *Tone Mapping Pass*. This technique is used to map colors properly to the monitor colors.|\-\-tone-mapping
render.line-width|double<br>1.0<br>render|Set the *width* of lines when showing edges.|\-\-line-width
render.show-edges|bool<br>false<br>render|Show the *cell edges*|\-\-edges
render.point-size|double<br>10.0<br>render|Set the *size* of points when showing vertices and point sprites.|\-\-point-size
render.point-sprites.enabled|bool<br>false<br>render|Show sphere *points sprites* instead of the geometry.|\-\-point-sprites
render.volume.enabled|bool<br>false<br>render|Enable *volume rendering*. It is only available for 3D image data (vti, dcm, nrrd, mhd files) and will display nothing with other default scene formats.|\-\-volume
render.volume.inverse|bool<br>false<br>render|Inverse the linear opacity function.|\-\-inverse
render.raytracing.denoise|bool<br>false<br>render|*Denoise* the raytracing rendering.|\-\-denoise
render.raytracing.enable|bool<br>false<br>render|Enable *raytracing*. Requires the raytracing module to be enabled.|\-\-raytracing
render.raytracing.samples|int<br>5<br>render|The number of *samples per pixel*.|\-\-samples

## UI Options

Option|Type<br>Default<br>Trigger|Description|F3D option
:---:|:---:|:---|:---:
ui.bar|bool<br>false<br>render|Show *scalar bar* of the coloring by data array.|\-\-bar
ui.cheatsheet|bool<br>false<br>render|Show a interactor cheatsheet
ui.filename|bool<br>false<br>render|Display the *name of the file*.|\-\-filename
ui.font-file|string<br>-<br>render|Use the provided FreeType compatible font file to display text.<br>Can be useful to display non-ASCII filenames.|\-\-font-file
ui.fps|bool<br>false<br>render|Display a *frame per second counter*.|\-\-fps
ui.loader-progress|bool<br>false<br>load|Show a *progress bar* when loading the file.|\-\-progress
ui.metadata|bool<br>false<br>render|Display the *metadata*.|\-\-metadata
