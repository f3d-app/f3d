# Interactions

## 3D Mouse Interactions

Simple interaction with the displayed data is possible directly within the window. It is as follows:

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

> Note: When playing an animation with a scene camera, camera interactions are locked.

## Bindings

The coloring can be controlled directly by pressing the following hotkeys:

- <kbd>A</kbd>: cycle between anti-aliasing modes.
- <kbd>C</kbd>: cycle between coloring with array from point data and from cell data.
- <kbd>S</kbd>: cycle the array to color with.
- <kbd>Y</kbd>: cycle the component of the array to color with.

See the [coloring cycle](#cycling-coloring) section for more info.

Other options can be toggled directly by pressing the following hotkeys:

- <kbd>W</kbd>: [cycle animations](ANIMATIONS.md#cycling-animations).
- <kbd>B</kbd>: display of the scalar bar, only when coloring and not using direct scalars.
- <kbd>V</kbd>: volume rendering, forces coloring.
- <kbd>I</kbd>: opacity function inversion during volume rendering.
- <kbd>O</kbd>: point sprites rendering.
- <kbd>P</kbd>: translucency support.
- <kbd>Q</kbd>: ambient occlusion.
- <kbd>T</kbd>: tone mapping.
- <kbd>E</kbd>: the display of cell edges.
- <kbd>X</kbd>: the trihedral axes display.
- <kbd>G</kbd>: the horizontal grid display.
- <kbd>Shift</kbd>+<kbd>x</kbd>: show/hide axes grid
- <kbd>N</kbd>: the display of the file name.
- <kbd>M</kbd>: the display of the metadata if exists.
- <kbd>Z</kbd>: the display of the FPS counter.
- <kbd>R</kbd>: raytracing.
- <kbd>D</kbd>: the denoiser when raytracing.
- <kbd>U</kbd>: background blur, useful with an HDRI skybox.
- <kbd>K</kbd>: trackball interaction mode.
- <kbd>F</kbd>: HDRI ambient lighting.
- <kbd>J</kbd>: the display of the HDRI skybox.
- <kbd>L</kbd>: increase lights intensity.
- <kbd>Shift</kbd>+<kbd>L</kbd>: decrease lights intensity.
- <kbd>Ctrl</kbd>+<kbd>P</kbd>: increase opacity.
- <kbd>Shift</kbd>+<kbd>P</kbd>: decrease opacity.
- <kbd>Shift</kbd>+<kbd>A</kbd>: toggle armature.

Note that the raytracing hotkeys are only available if F3D is build with raytracing enabled.

Camera Hotkeys:

- <kbd>1</kbd>: front view camera.
- <kbd>2</kbd>: roll the camera up (elevation) by 90 degrees.
- <kbd>3</kbd>: right view camera.
- <kbd>4</kbd>: roll the camera left by 90 degrees.
- <kbd>5</kbd>: toggle orthographic projection.
- <kbd>6</kbd>: roll the camera right by 90 degrees.
- <kbd>7</kbd>: top view camera.
- <kbd>8</kbd>: roll the camera down (elevation) by 90 degrees.
- <kbd>9</kbd>: isometric view camera.
- <kbd>Enter</kbd>: reset the camera to its initial parameters.

Other hotkeys and interactions are available:

- <kbd>H</kbd>: key to toggle the display of a cheat sheet showing all these hotkeys and their statuses.
- <kbd>CTRL</kbd> + <kbd>Q</kbd>: close the window and quit F3D.
- <kbd>Esc</kbd>: display the console or hide console/minimal console.
- <kbd>:</kbd>: display the minimal console
- <kbd>Space</kbd>: play the animation if any.
- <kbd>Shift</kbd> + <kbd>V</kbd>: cycle between verbosity levels.
- <kbd>&larr;</kbd>: load the previous file if any and reset the camera.
- <kbd>&rarr;</kbd>: load the next file if any and reset the camera.
- <kbd>&uarr;</kbd>: reload the currently loaded files.
- <kbd>&darr;</kbd>: add all current files parent directories to the list of files, reload the currently loaded files and reset the camera, respect the `--recursive-dir-add` CLI option.
- <kbd>F12</kbd>: take a screenshot, ie. render the current view to an image file.
- <kbd>Ctrl</kbd> + <kbd>F12</kbd>: take a "minimal" screenshot, ie. render the current view with no grid and no overlays to an image file with a transparent background.
- _Drop_ a file, directory or HDRI into the F3D window to load it. F3D relies on the extension (`.hdr` or `.exr`) to detect if the dropped file is an HDRI.
- <kbd>Ctrl</kbd> + _Drop_ a file, directory and load it.
- <kbd>Shift</kbd> + _Drop_ a HDRI and use it, no extensions checks performed.

When loading another file or reloading, options that have been changed interactively are kept as is, except when using [imperative options](CONFIGURATION_FILE.md#imperative-options) in configuration files.

## Cycling Coloring

The following hotkeys let you cycle the coloring of the data:

- <kbd>C</kbd>: cycle between point data and cell data - field data is not supported.
- <kbd>S</kbd>: cycle the array available on the currently selected data in alphabetical order,
  skipping array not containing numeric data. It will loop back to not coloring unless using volume rendering.
- <kbd>Y</kbd>: cycle the component available on the currently selected array, looping to -2 for direct scalars rendering.

When changing the array, the component in use will be kept if valid with the new array, if not it will be cycled until a valid
component is found.
When changing the type of data to color with, the index of the array within the data will be kept if valid
with the new data. If not, it will cycle until a valid array is found. After that, the component will be checked
as specified above.

## Taking Screenshots

The destination filename used to save the screenshots (created by pressing <kbd>F12</kbd> or <kbd>Ctrl</kbd> + <kbd>F12</kbd>) is configurable (using the `screenshot-filename` option) and can use template variables as described [on the options page](OPTIONS.md#filename-templating).

Unless the configured filename template is an absolute path, images will be saved in dedicated directories:

- Windows: `%USERPROFILE%\Pictures`
- Linux: `${XDG_PICTURES_DIR}`,`${HOME}`
- macOS: `${HOME}`

So with default options, which are using [templates](OPTIONS.md#filename-templating), the screenshots will be saved as:

- Windows: `%USERPROFILE%\Pictures\F3D\{model}_{n}.png`
- Linux: `${XDG_PICTURES_DIR}/F3D/{model}_{n}.png`,`${HOME}/F3D/{model}_{n}.png`
- macOS: `${HOME}/F3D/{model}_{n}.png`

## Configuring bindings

It is supported to change all bindings above using a specific syntax in the [configuration file](CONFIGURATION_FILE.md) using [commands](COMMANDS.md).
