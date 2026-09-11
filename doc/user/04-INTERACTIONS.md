# Interactions

## 3D mouse interactions

Interact with the displayed data directly in the window:

- _Click and drag_ with the _left_ mouse button to rotate around the focal point of the camera.
- Hold <kbd>Shift</kbd> then _Click and drag_ with the _left_ mouse button to translate the camera.
- Hold <kbd>Ctrl</kbd> then _Click and drag_ with the _left_ mouse button to rotate around the axis of the camera.
- Hold <kbd>Shift</kbd> then _Click and drag_ horizontally with the _right_ mouse button to rotate the HDRI.
- _Click and drag_ vertically with the _right_ mouse button to zoom in/out.
- _Move the mouse wheel_ to zoom in/out.
- Hold <kbd>Ctrl</kbd>, then _Click and drag_ vertically with the _right_ mouse button to zoom in/out to mouse position.
- Hold <kbd>Ctrl</kbd>, then _Move the mouse wheel_ to zoom in/out to mouse position.
- _Click and drag_ with the _middle_ mouse button to translate the camera.
- _Click_ with the _middle_ mouse button to center the camera on the point under the cursor (hold <kbd>Shift</kbd> to allow forward or backward movement).

> [!NOTE]
> When playing an animation with a scene camera, camera interactions are locked.

## 2D mode

F3D supports a 2D interaction mode designed for viewing images and flat content. In 2D mode:

- Rotation is disabled
- Camera commands (`set_camera`, `roll_camera`, `elevation_camera`, `azimuth_camera`) are disabled
- _Click and drag_ with the _left_ or _middle_ mouse button to pan
- _Click and drag_ with the _right_ mouse button or use the _mouse wheel_ to zoom
- The default camera zoom factor is `1.0` (no padding) instead of `0.9`

Select 2D interaction styles by pressing <kbd>K</kbd> or through the `--interaction-style=2d` CLI option.

## Bindings

Control coloring directly with these hotkeys:

- <kbd>A</kbd>: cycle between anti-aliasing modes.
- <kbd>C</kbd>: cycle between coloring with array from point data and from cell data.
- <kbd>S</kbd>: cycle the array to color with.
- <kbd>Y</kbd>: cycle the component of the array to color with.

See the [coloring cycle](#cycle-coloring) section for more info.

Toggle or cycle other options directly with these hotkeys:

- <kbd>W</kbd>: [cycle animations](05-ANIMATIONS.md#cycle-animations).
- <kbd>B</kbd>: display the Scalar bar, only when coloring and not using direct scalars.
- <kbd>V</kbd>: toggle Volume rendering, which forces coloring.
- <kbd>I</kbd>: toggle Inverse volume opacity during volume rendering.
- <kbd>O</kbd>: cycle Point sprites.
- <kbd>P</kbd>: cycle the Blending mode for translucency support.
- <kbd>Q</kbd>: toggle Ambient occlusion.
- <kbd>T</kbd>: toggle tone mapping.
- <kbd>E</kbd>: toggle edges display.
- <kbd>X</kbd>: toggle axes display.
- <kbd>G</kbd>: toggle grid display.
- <kbd>Shift</kbd>+<kbd>X</kbd>: toggle axes grid display.
- <kbd>N</kbd>: toggle Filename.
- <kbd>Shift</kbd>+<kbd>N</kbd>: toggle HDRI filename.
- <kbd>M</kbd>: toggle Metadata, if it exists.
- <kbd>Shift</kbd>+<kbd>H</kbd>: toggle Scene hierarchy.
- <kbd>Z</kbd>: toggle FPS Counter.
- <kbd>R</kbd>: toggle Raytracing rendering.
- <kbd>D</kbd>: toggle Denoising when raytracing.
- <kbd>U</kbd>: toggle Blur background, useful with an HDRI skybox.
- <kbd>K</kbd>: cycle Interaction style.
- <kbd>F</kbd>: toggle HDRI ambient lighting.
- <kbd>J</kbd>: toggle HDRI skybox.
- <kbd>L</kbd>: increase lights intensity.
- <kbd>Shift</kbd>+<kbd>L</kbd>: decrease lights intensity.
- <kbd>Ctrl</kbd>+<kbd>P</kbd>: increase opacity.
- <kbd>Shift</kbd>+<kbd>P</kbd>: decrease opacity.
- <kbd>Shift</kbd>+<kbd>A</kbd>: toggle Armature.
- <kbd>Ctrl</kbd>+<kbd>Y</kbd>: set scene up direction to +Y.
- <kbd>Ctrl</kbd>+<kbd>Z</kbd>: set scene up direction to +Z.

Raytracing hotkeys are only available if F3D is build with raytracing enabled.

Camera hotkeys:

- <kbd>1</kbd>: set Front View camera.
- <kbd>2</kbd>: rotate camera down by 90 degrees.
- <kbd>3</kbd>: set Right View camera.
- <kbd>4</kbd>: rotate camera right by 90 degrees.
- <kbd>5</kbd>: toggle Orthographic Projection.
- <kbd>6</kbd>: rotate camera left by 90 degrees.
- <kbd>7</kbd>: set Top View camera.
- <kbd>8</kbd>: rotate camera up by 90 degrees.
- <kbd>9</kbd>: set Isometric View camera.
- <kbd>Enter</kbd>: reset camera to initial parameters.

Scene hotkeys:

- <kbd>Ctrl</kbd>+<kbd>Y</kbd>: set scene up direction to +Y.
- <kbd>Ctrl</kbd>+<kbd>Z</kbd>: set scene up direction to +Z.

Use these other hotkeys and interactions:

- <kbd>H</kbd>: toggle Cheatsheet.
- <kbd>Ctrl</kbd>+<kbd>Q</kbd>: quit F3D.
- <kbd>Esc</kbd>: toggle Console, or hide Console/Minimal console.
- <kbd>:</kbd>: toggle Minimal console.
- <kbd>Ctrl</kbd>+<kbd>K</kbd>: toggle Notifications.
- <kbd>Space</kbd>: play/pause animation if any.
- <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>Space</kbd>: play/pause animation backward if any.
- <kbd>Shift</kbd>+<kbd>V</kbd>: cycle Verbose level.
- <kbd>&larr;</kbd>: load previous file group, if any, and reset the camera.
- <kbd>Ctrl</kbd>+<kbd>&larr;</kbd>: load previous file group, if any, keeping camera.
- <kbd>&rarr;</kbd>: load next file group, if any, and reset the camera.
- <kbd>Ctrl</kbd>+<kbd>&rarr;</kbd>: load next file group, if any, keeping camera.
- <kbd>&uarr;</kbd>: reload current file group.
- <kbd>&darr;</kbd>: add files from dir of current file, reload the current file group, and reset the camera. The `--recursive-dir-add` CLI option controls recursive directory loading.
- <kbd>F12</kbd>: take a screenshot, ie. render the current view to an image file.
- <kbd>Ctrl</kbd>+<kbd>F12</kbd>: take a minimal screenshot, ie. render the current view with no grid and no overlays to an image file with a transparent background.
- <kbd>Ctrl</kbd>+<kbd>S</kbd>: save a statefile (file dialog). Requires a build with the `tinyfiledialogs` module.
- <kbd>Ctrl</kbd>+<kbd>L</kbd>: load a statefile (file dialog). Requires a build with the `tinyfiledialogs` module. Does nothing but warn if the file does not exist.
- <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>S</kbd>: save a statefile (auto filename). Uses the statefile set by the `--statefile-filename` CLI option, or an automatic filename (`{app}/{model}_{n}.json`) when it is not set.
- <kbd>Ctrl</kbd>+<kbd>Shift</kbd>+<kbd>L</kbd>: load the last auto-filename statefile. Uses the statefile set by the `--statefile-filename` CLI option, or the most recent statefile saved with the automatic filename when it is not set.
- <kbd>Ctrl</kbd>+<kbd>C</kbd>: save a statefile to the clipboard. Requires a build with the `clip` module.
- <kbd>Ctrl</kbd>+<kbd>V</kbd>: load a statefile from the clipboard. Requires a build with the `clip` module.
- <kbd>Drop</kbd>: load dropped files, folder or HDRI. F3D relies on the extension (`.hdr` or `.exr`) to detect if the dropped file is an HDRI.
- <kbd>Ctrl</kbd>+<kbd>Drop</kbd>: load dropped files or folder.
- <kbd>Shift</kbd>+<kbd>Drop</kbd>: set HDRI and use it. No extension checks are performed.

When loading another file or reloading, F3D keeps options that have been changed interactively, except when using [imperative options](06-CONFIGURATION_FILE.md#imperative-options) in configuration files.

## Cycle coloring

The following hotkeys let you cycle the coloring of the data:

- <kbd>C</kbd>: cycle between point data and cell data - field data is not supported.
- <kbd>S</kbd>: cycle the array available on the currently selected data in alphabetical order,
  skipping array not containing numeric data. It loops back to not coloring unless using volume rendering.
- <kbd>Y</kbd>: cycle the component available on the currently selected array, looping to -2 for direct scalars rendering.

When changing the array, F3D keeps the component in use if it is valid with the new array. Otherwise, it cycles until a valid
component is found.
When changing the type of data to color with, F3D keeps the index of the array within the data if it is valid
with the new data. Otherwise, it cycles until a valid array is found. After that, F3D checks the component
as specified above.

## Take screenshots

Configure the destination filename used to save screenshots with the `screenshot-filename` option. Screenshots are created by pressing <kbd>F12</kbd> or <kbd>Ctrl</kbd>+<kbd>F12</kbd>. The filename can use template variables as described [on the options page](03-OPTIONS.md#filename-templating).

Unless the configured filename template is an absolute path, F3D saves images in dedicated directories:

- Windows: `%USERPROFILE%\Pictures`
- Linux: `${XDG_PICTURES_DIR}`,`${HOME}`
- macOS: `${HOME}`

With default options, which use [templates](03-OPTIONS.md#filename-templating), F3D saves screenshots as:

- Windows: `%USERPROFILE%\Pictures\F3D\{model}_{n}.png`
- Linux: `${XDG_PICTURES_DIR}/F3D/{model}_{n}.png`,`${HOME}/F3D/{model}_{n}.png`
- macOS: `${HOME}/F3D/{model}_{n}.png`

## Configure bindings

Change the bindings above with a specific syntax in the [configuration file](06-CONFIGURATION_FILE.md) using [commands](07-COMMANDS.md).

## macOS bindings

When using F3D on macOS, all bindings that specify <kbd>Ctrl</kbd> and <kbd>Ctrl</kbd>+<kbd>Shift</kbd> use <kbd>Cmd</kbd> and <kbd>Cmd</kbd>+<kbd>Shift</kbd> respectively instead.
