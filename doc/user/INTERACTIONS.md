# Interactions

## Mouse Interactions

Simple interaction with the displayed data is possible directly within the window. It is as follows:

* *Click and drag* with the *left* mouse button to rotate around the focal point of the camera.
* Hold *Shift* then *Click and drag* horizontally with the *right* mouse button to rotate the HDRI.
* *Click and drag* vertically with the *right* mouse button to zoom in/out.
* *Move the mouse wheel* to zoom in/out.
* *Click and drag* with the *middle* mouse button to translate the camera.
* *Click* with the *middle* mouse button to center the camera on the point under the cursor (hold *Shift* to allow forward or backward movement).
* Drag and drop a file or directory into the F3D window to load it

> Note: When playing an animation with a scene camera, camera interactions are locked.

## Hotkeys

The coloring can be controlled directly by pressing the following hotkeys:

* `C`: cycle between coloring with array from point data and from cell data.
* `S`: cycle the array to color with.
* `Y`: cycle the component of the array to color with.

See the [coloring cycle](#cycling-coloring) section for more info.

Other options can be toggled directly by pressing the following hotkeys:

* `B`: display of the scalar bar, only when coloring and not using direct scalars.
* `V`: volume rendering.
* `I`: opacity function inversion during volume rendering.
* `O`: point sprites rendering.
* `P`: translucency support.
* `Q`: ambient occlusion.
* `A`: anti-aliasing.
* `T`: tone mapping.
* `E`: the display of cell edges.
* `X`: the trihedral axes display.
* `G`: the horizontal grid display.
* `N`: the display of the file name.
* `M`: the display of the metadata if exists.
* `Z`: the display of the FPS counter.
* `R`: raytracing.
* `D`: the denoiser when raytracing.
* `U`: background blur when using a HDRi.
* `K`: trackball interaction mode.
* `L`: increase (+Shift: decrease) lights intensity.

Note that some hotkeys can be available or not depending on the file being loaded and the F3D configuration.

Other hotkeys are available:

* `H`: key to toggle the display of a cheat sheet showing all these hotkeys and their statuses.
* `?`: key to print scene description to the terminal.
* `ESC`: close the window and quit F3D.
* `ENTER`: reset the camera to its initial parameters.
* `SPACE`: play the animation if any.
* `LEFT`: load the previous file if any.
* `RIGHT`: load the next file if any.
* `UP`: reload the current file.
* `DOWN`: add current file parent directory to the list of files and reload the current file.

When loading another file or reloading, options that have been changed interactively are kept but can be overridden
if a dedicated regular expression block in the configuration file is present, see the [configuration file](CONFIGURATION_FILE.md)
documentation for more info.

## Cycling Coloring

When using the default scene, the following hotkeys let you cycle the coloring of the data:

* `C`: cycle between point data and cell data - field data is not supported.
* `S`: cycle the array available on the currently selected data in alphabetical order,
skipping array not containing numeric data. It will loop back to not coloring unless using volume rendering.
* `Y`: cycle the component available on the currently selected array, looping to -2 for direct scalars rendering.

When changing the array, the component in use will be kept if valid with the new array, if not it will be cycled until a valid
component is found.
When changing the type of data to color with, the index of the array within the data will be kept if valid
with the new data. If not, it will cycle until a valid array is found. After that, the component will be checked
as specified above.
