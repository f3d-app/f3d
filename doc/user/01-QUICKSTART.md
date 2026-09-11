# Quickstart guide

Use this guide to get started with F3D.

Learn how to run F3D, open files, configure basic scene constructions, interact with them, and play animations.

## Prerequisites

Install F3D first. See the download page for the latest release package supported by your system.

## Run F3D

After installing F3D, open a file in one of these ways:

- Open a file directly from your file manager.
- Open F3D directly, then drag and drop a file into the F3D window.
- Run F3D from the terminal with a set of command-line [options](03-OPTIONS.md).
- Configure F3D as a [thumbnailer](12-DESKTOP_INTEGRATION.md) for supported file formats with certain file managers.

To run F3D from the terminal, open your file with:

```
f3d /path/to/file.ext
```

Optionally, save the rendering into an image file:

```
f3d /path/to/file.ext --output=/path/to/img.png
```

If you need help, specify the `--help` [option](03-OPTIONS.md):

```
f3d --help
man f3d # Linux only
```

After opening your file in F3D, press <kbd>H</kbd> to open a list of shortcuts.

See the [supported file formats](02-SUPPORTED_FORMATS.md) page for the full list of file formats that F3D can open.

## Construct scenes

The [supported file formats](02-SUPPORTED_FORMATS.md) page lists certain formats as **full scene** formats (.gltf/.glb, .3ds, .wrl, .obj, .fbx, .dae, .off, .x, .3mf, .usd).
These files contain _geometry_ as well as scene information such as _lights_, _cameras_, _actors_,
and _texture_ properties. By default, F3D loads this information from the file and displays it.
For file formats that are not **full scene**, F3D creates **a default scene**.

For **default scene** formats, certain default values are set automatically:

- texture-\*: ""
- line-width: 1.0
- point-size: 10.0
- opacity: 1.0
- color: 0.65, 0.65, 0.65
- emissive-factor: 1.0, 1.0, 1.0
- normal-scale: 1.0
- metallic: 0.0
- roughness: 0.3
- base-ior: 1.5
- camera-orthographic: false
- unlit: false

Corresponding [options](03-OPTIONS.md) override these defaults, either from command-line options or from the [configuration file](06-CONFIGURATION_FILE.md).

## Interact with your scene

After loading your scene in F3D, interact with it using your mouse and hotkeys.

### Moving the camera

- _Click and drag_ with the _left_ mouse button to **rotate** around the focal point of the camera.
- _Click and drag_ vertically with the _right_ mouse button <u>OR</u> _move the mouse wheel_ to **zoom in/out**.
- _Click and drag_ with the _middle_ mouse button to **translate** the camera.

### Other shortcuts

- Press <kbd>Enter</kbd> to reset the camera.
- Press <kbd>Space</kbd> to play animation, if any.
- Press <kbd>G</kbd> to toggle the horizontal grid.
- Press <kbd>H</kbd> to display a cheatsheet of hotkey interactions.

For more information, see the [Interactions](04-INTERACTIONS.md) page.

## Play animations

F3D can play [animations](05-ANIMATIONS.md) for any [supported files](02-SUPPORTED_FORMATS.md) that contain them.

With your file loaded in F3D, press the <kbd>W</kbd> hotkey to cycle through available animations. Then press <kbd>Space</kbd> to play your selected animation.

Alternatively, use the command line to play animations. Use the `--animation-indices` [command-line option](03-OPTIONS.md) to specify which animation to play. To play all animations at once, use `--animation-indices=-1`.

For more information, see the [Animations](05-ANIMATIONS.md) page.

## Further reading

- [A list of all F3D command-line options.](03-OPTIONS.md)
- [How to interact with scenes in F3D.](04-INTERACTIONS.md)
- [How to play animations in F3D.](05-ANIMATIONS.md)
- [How to specify colormaps in F3D.](10-COLOR_MAPS.md)
- [How to configure plugins in F3D.](13-PLUGINS.md)
- [Limitations and how to troubleshoot F3D.](14-LIMITATIONS_AND_TROUBLESHOOTING.md)
