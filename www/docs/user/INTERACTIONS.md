---
sidebar_position: 4
---

# Interactions

## Mouse Interactions

Simple interaction with the displayed data is possible directly within the window. It is as follows:

* *Click and drag* with the *left* mouse button to rotate around the focal point of the camera.
* Hold <kbd>Shift</kbd> then *Click and drag* horizontally with the *right* mouse button to rotate the HDRI.
* *Click and drag* vertically with the *right* mouse button to zoom in/out.
* *Move the mouse wheel* to zoom in/out.
* Hold <kbd>Ctrl</kbd>, then *Click and drag* vertically with the *right* mouse button to zoom in/out to mouse position.
* Hold <kbd>Ctrl</kbd>, then *Move the mouse wheel* to zoom in/out to mouse position.
* *Click and drag* with the *middle* mouse button to translate the camera.
* *Click* with the *middle* mouse button to center the camera on the point under the cursor (hold <kbd>Shift</kbd> to allow forward or backward movement).
* Drag and drop a file, directory or HDRI into the F3D window to load it. F3D relies on the extension (`.hdr` or `.exr`) to detect if the dropped file is an HDRI.

> Note: When playing an animation with a scene camera, camera interactions are locked.

## Hotkeys

The coloring can be controlled directly by pressing the following hotkeys:

* <kbd>C</kbd>: cycle between coloring with array from point data and from cell data.
* <kbd>S</kbd>: cycle the array to color with.
* <kbd>Y</kbd>: cycle the component of the array to color with.

See the [coloring cycle](#cycling-coloring) section for more info.

Other options can be toggled directly by pressing the following hotkeys:

* <kbd>B</kbd>: display of the scalar bar, only when coloring and not using direct scalars.
* <kbd>V</kbd>: volume rendering.
* <kbd>I</kbd>: opacity function inversion during volume rendering.
* <kbd>O</kbd>: point sprites rendering.
* <kbd>P</kbd>: translucency support.
* <kbd>Q</kbd>: ambient occlusion.
* <kbd>A</kbd>: anti-aliasing.
* <kbd>T</kbd>: tone mapping.
* <kbd>E</kbd>: the display of cell edges.
* <kbd>X</kbd>: the trihedral axes display.
* <kbd>G</kbd>: the horizontal grid display.
* <kbd>N</kbd>: the display of the file name.
* <kbd>M</kbd>: the display of the metadata if exists.
* <kbd>Z</kbd>: the display of the FPS counter.
* <kbd>R</kbd>: raytracing.
* <kbd>D</kbd>: the denoiser when raytracing.
* <kbd>U</kbd>: background blur, useful with an HDRI skybox.
* <kbd>K</kbd>: trackball interaction mode.
* <kbd>F</kbd>: HDRI ambient lighting.
* <kbd>J</kbd>: the display of the HDRI skybox.
* <kbd>L</kbd>: increase lights intensity.
* <kbd>Shift</kbd>+<kbd>L</kbd>: decrease lights intensity.

Note that some hotkeys can be available or not depending on the file being loaded and the F3D configuration.

Camera Hotkeys:
* <kbd>1</kbd>: front view camera.
* <kbd>3</kbd>: right view camera.
* <kbd>7</kbd>: top view camera.
* <kbd>9</kbd>: isometric view camera.
* <kbd>Enter</kbd>: reset the camera to its initial parameters.

Other hotkeys are available:

* <kbd>H</kbd>: key to toggle the display of a cheat sheet showing all these hotkeys and their statuses.
* <kbd>?</kbd>: key to print scene description to the terminal.
* <kbd>Esc</kbd>: close the window and quit F3D.
* <kbd>Space</kbd>: play the animation if any.
* <kbd>&larr;</kbd>: load the previous file if any.
* <kbd>&rarr;</kbd>: load the next file if any.
* <kbd>&uarr;</kbd>: reload the current file without resetting the camera.
* <kbd>&darr;</kbd>: add current file parent directory to the list of files, reload the current file and reset the camera.

When loading another file or reloading, options that have been changed interactively are kept but can be overridden
if a dedicated regular expression block in the configuration file is present, see the [configuration file](CONFIGURATION_FILE.md)
documentation for more info.

## Cycling Coloring

When using the default scene, the following hotkeys let you cycle the coloring of the data:

* <kbd>C</kbd>: cycle between point data and cell data - field data is not supported.
* <kbd>S</kbd>: cycle the array available on the currently selected data in alphabetical order,
skipping array not containing numeric data. It will loop back to not coloring unless using volume rendering.
* <kbd>Y</kbd>: cycle the component available on the currently selected array, looping to -2 for direct scalars rendering.

When changing the array, the component in use will be kept if valid with the new array, if not it will be cycled until a valid
component is found.
When changing the type of data to color with, the index of the array within the data will be kept if valid
with the new data. If not, it will cycle until a valid array is found. After that, the component will be checked
as specified above.
