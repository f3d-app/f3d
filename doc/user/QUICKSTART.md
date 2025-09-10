# Quickstart Guide

This guide will help you get started using F3D.

As an overview, you'll learn how to run F3D and open files, configure basic scene constructions, interact with them, and play animations.

## Prerequisites

To use F3D, you'll need to install F3D. See the [Installation](INSTALLATION.md) page for the latest release package supported by your system.

## Running F3D

Once F3D has been installed, you should be able to open a file in any of the following ways:

- Open a file directly from your file manager.
- Open the F3D application directly, then drag and drop a file into the application window.
- Run F3D from the terminal with a set of command-line [options](OPTIONS.md).
- Configure F3D as a [thumbnailer](DESKTOP_INTEGRATION.md) for supported file formats with certain file managers.

If you choose to run F3D from the terminal, you can open your file by running the following:

```
f3d /path/to/file.ext
```

Optionally, you can also save the rendering into an image file:

```
f3d /path/to/file.ext --output=/path/to/img.png
```

If you need help, specify the `--help` [option](OPTIONS.md):

```
f3d --help
man f3d # Linux only
```

Once you've opened your file in F3D, you're all set to start visualizing! Press <kbd>H</kbd> to open a list of shortcuts to help you interact with your scene.

See the [supported file formats](SUPPORTED_FORMATS.md) page for the full list of file types that F3D can open.

## Constructing scenes

Certain [formats](SUPPORTED_FORMATS.md) are listed as **full scene** formats (.gltf/.glb, .3ds, .wrl, .obj, .fbx, .dae, .off, .x, .3mf, .usd)
which means these files contain not only _geometry_ but also some scene information like _lights_, _cameras_, _actors_ in the scene,
as well as _texture_ properties. By default, all this information will be loaded from the file and displayed.
For file formats that are not **full scene**, **a default scene** is created.

For **default scene** formats, certain default values are set automatically:

- texture-\*: ""
- line-width: 1.0
- point-size: 10.0
- opacity: 1.0
- color: 1.0, 1.0, 1.0
- emissive-factor: 1.0, 1.0, 1.0
- normal-scale: 1.0
- metallic: 0.0
- roughness: 0.3
- base-ior: 1.5
- camera-orthographic: false

They will be overridden when using corresponding [options](OPTIONS.md), either from command line or [configuration file](CONFIGURATION_FILE.md).

## Interacting with your scene

Once you've loaded your scene in F3D, you can interact with it by using your mouse and certain hotkeys.

### Moving the camera

- _Click and drag_ with the _left_ mouse button to **rotate** around the focal point of the camera.
- _Click and drag_ vertically with the _right_ mouse button <u>OR</u> _move the mouse wheel_ to **zoom in/out**.
- _Click and drag_ with the _middle_ mouse button to **translate** the camera.

### Other shortcuts

- Press <kbd>Enter</kbd> to reset the camera.
- Press <kbd>Space</kbd> to play animation, if any.
- Press <kbd>G</kbd> to toggle the horizontal grid.
- Press <kbd>H</kbd> to display a cheatsheet of hotkey interactions.

For more information, see the [Interactions](INTERACTIONS.md) page.

## Playing animations

F3D can play [animations](ANIMATIONS.md) for any [supported files](SUPPORTED_FORMATS.md) that contain them.

With your file loaded into F3D, press the <kbd>W</kbd> hotkey to cycle through available animations. Then, press <kbd>Space</kbd> to play your selected animation.

Alternatively, you can use the command line to play animations. Use the `--animation-indices` [option](OPTIONS.md) to specify which animation you want to play. To play all animations at once, use `--animation-indices=-1`.

For more information, see the [Animations](ANIMATIONS.md) page.

## Further Reading

- [A list of all F3D command-line options.](OPTIONS.md)
- [How to interact with scenes in F3D.](INTERACTIONS.md)
- [How to play animations in F3D.](ANIMATIONS.md)
- [How to specify colormaps in F3D.](COLOR_MAPS.md)
- [How to configure plugins in F3D.](PLUGINS.md)
- [Limitations and how to troubleshoot F3D.](LIMITATIONS_AND_TROUBLESHOOTING.md)
