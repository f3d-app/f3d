# Quickstart Guide

This guide will help you get started using F3D.

As an overview, you'll learn how to run F3D and open files, configure basic scene constructions, interact with them, and play animations.

## Prerequisites

To use F3D, you'll need to install F3D. See the [Installation](INSTALLATION.md) page for the latest release package supported by your system.

## Running F3D

Once F3D has been installed, you should be able to open a file in any of the following ways:

* Open a file directly from your file manager.
* Open F3D, then drag and drop a file into the application window.
* Run F3D from the terminal with a set of command-line [options](OPTIONS.md).
* Configure F3D as a [thumbnailer](DESKTOP_INTEGRATION.md) for supported file formats with certain file managers.

If you choose to run F3D from the terminal, you have different options.

1\. Open a file and visualize it interactively:

```
f3d /path/to/file.ext
```

2\. Open a file and save the rendering into an image file:

```
f3d /path/to/file.ext --output=/path/to/img.png
```

If you need help, specify the `--help` [option](OPTIONS.md):

```
f3d --help
man f3d # Linux only
```

> Note: See the [supported file formats](SUPPORTED_FORMATS.md) page for the full list of file types that F3D can open.

## Constructing scenes

For most file formats, F3D constructs **a default scene**, loading only the *geometry* from the file.

However, formats supporting **full scenes** will automatically load additional scene properties including *lights*, *cameras*, *actors* and *texture*.

If you'd like to disable these properties, you can specify the `--geometry-only` [option](OPTIONS.md) from the command line to construct a default scene instead.

See the table of [supported file formats](SUPPORTED_FORMATS.md) if you're unsure about what kind of scene your file supports.

## Interacting with your scene

Once you've loaded your scene in F3D, you can interact with it by using your mouse and certain hotkeys. 

### Moving the camera

* *Click and drag* with the *left* mouse button to **rotate** around the focal point of the camera.
* *Click and drag* vertically with the *right* mouse button <u>OR</u> *move the mouse wheel* to **zoom in/out**.
* *Click and drag* with the *middle* mouse button to **translate** the camera.

### Other shortcuts
* Press <kbd>Enter</kbd> to reset the camera.
* Press <kbd>Space</kbd> to play animation, if any.
* Press <kbd>G</kbd> to toggle the horizontal grid.
* Press <kbd>H</kbd> to display a cheatsheet of hotkey interactions. 

For more information, see the [Interactions](INTERACTIONS.md) page.

## Playing animations

F3D can play [animations](ANIMATIONS.md) for any [supported files](SUPPORTED_FORMATS.md) that contain them.

With your file loaded into F3D, press the <kbd>W</kbd> hotkey to cycle through available animations. Then, press <kbd>Space</kbd> to play your selected animation. 

Alternatively, you can use the command line to play animations. Use the `--animation-index` [option](OPTIONS.md) to specify which animation you want to play. To play all animations at once, use `--animation-index=-1` (.gltf/.glb only). 

A few things to note about F3D's default animation settings:
* F3D plays animations at a default speed factor of 1. Change the `--animation-speed-factor` [option](OPTIONS.md) to adjust how fast or slow your animation plays.
* Additionally, F3D animates at a default frame rate of 60 frames per second. Change the `--animation-frame-rate` [option](OPTIONS.md) to adjust your desired frame rate.

For more information, see the [Animations](ANIMATIONS.md) page.

## Further Reading

* [A list of all F3D command-line options.](OPTIONS.md)
* [How to interact with scenes in F3D.](INTERACTIONS.MD)
* [How to play animations in F3D.](ANIMATIONS.md)
* [How to specify colormaps in F3D.](COLOR_MAPS.md)
* [How to configure plugins in F3D.](PLUGINS.md)
* [Limitations and how to troubleshoot F3D.](LIMITATIONS_AND_TROUBLESHOOTING.md)
