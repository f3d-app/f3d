# Quickstart Guide

This guide will help you get started using F3D.

As an overview, you'll learn how to run F3D and open supported files. You'll then learn how to configure basic scene constructions, animations and plugins.

## Prerequisites

To use F3D, you'll need to install F3D. See the [Installation](INSTALLATION.md) page for the latest release package supported by your system.

## Running F3D

Once F3D has been installed, you should be able to open a [supported file](#supported-file-formats) in any of the following ways:

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

## Supported file formats

F3D supports the following file formats:

* Legacy VTK (`.vtk`)
* VTK XML (`.vtp`, `.vtu`, `.vtr`, `.vti`, `.vts`, `.vtm`)
* Polygon File Format (`.ply`)
* OBJ (`.obj`)
* STL (`.stl`)
* STEP (`.step`, `.stp`)
* IGES (`.iges`, `.igs`)
* Filmbox (`.fbx`): animations supported
* Autodesk 3D Studio (`.3ds`): full scene by default
* VDB (`.vdb`): experimental as of version 2.5.0

See the [supported file formats](SUPPORTED_FILE_FORMATS.md) page for a full list of compatible file types.

## Constructing scenes

For most file formats, F3D constructs **a default scene**, loading only the *geometry* from the file.

However, formats supporting **full scenes** will automatically load additional scene properties including *lights*, *cameras*, *actors* and *texture*.

If you'd like to disable these properties, you can specify the `--geometry-only` [option](OPTIONS.md) from the command line to construct a default scene instead.

See the table of [supported file formats](SUPPORTED_FILE_FORMATS.md) if you're unsure about what kind of scene your file supports.

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

F3D can play [animations](ANIMATIONS.md) for any [supported files](#supported-file-formats) that contain them.

With your file loaded into F3D, press the <kbd>W</kbd> hotkey to cycle through available animations. Then, press <kbd>Space</kbd> to play your selected animation. 

Alternatively, you can use the command line to play animations. Use the `--animation-index` [option](OPTIONS.md) to specify which animation you want to play. To play all animations at once, use `--animation-index=-1` (.gltf/.glb only). 

A few things to note about F3D's default animation settings:
* F3D plays animations at a default speed factor of 1. Change the `--animation-speed-factor` [option](OPTIONS.md) to adjust how fast or slow your animation plays.
* Additionally, F3D animates at a default frame rate of 60 frames per second. Change the `--animation-frame-rate` [option](OPTIONS.md) to adjust your desired frame rate.

For more information, see the [Animations](ANIMATIONS.md) page.

## Plugins

F3D officially supports [plugins](#supported-plugins) for certain file formats. If you installed F3D using the binary release, there's no need to manually load these plugins when opening your file. F3D will load them automatically.

However, if you installed F3D using a package manager, it's possible that the packager chose to bundle the plugins into different packages or to list plugin dependencies as optional in order to reduce the number of dependencies of the main package.

In order to open a file that requires a plugin, make sure you've installed all necessary dependencies. You can then specify the `--load-plugins=<path or name>` [option](OPTIONS.md) in the command line to load your plugin. 

Alternatively, you can add your plugin directly in the [configuration file](CONFIGURATION_FILE.md) if it isn't there already. You can specify one or multiple plugins in a single comma-separated list, like in the example below:

```
{
    ".*(file_extension)": {
        "load-plugins": "plugin_name"
    }
}
```

### Supported plugins

F3D supports the following plugins and their file formats:

- **alembic**: `.abc`
- **assimp**: `.fbx`, `.dae`, `.off`, `.dxf`, `.x`, `.3mf`
- **draco**: `.drc`
- **exodus**: `.ex2`
- **occt**: `.step/.stp`, `.iges/.igs`
- **usd**: `.usd`, `.usda`, `.usdc`, `.usdz`
- **vdb**: `.vdb` (experimental)

> Note: If you downloaded the binaries from the release page, it's not necessary to specify manually the plugins above. F3D loads them automatically.

Here is how the plugins are searched (in preceding order):

1. Search the static plugins.
2. Consider the option given is a full path.
3. Search in the paths specified in `F3D_PLUGINS_PATH` environment variable.
4. Search in a directory relative to the F3D application: `../lib`.
5. Rely on OS specific paths (e.g. `LD_LIBRARY_PATH` on Linux or `DYLD_LIBRARY_PATH` on macOS).

You can also try plugins maintained by the community. If you have created a plugin and would like it to be listed here, please submit a pull request.

- **Abaqus**: ODB support by @YangShen398 ([repository](https://github.com/YangShen398/F3D-ODB-Reader-Plugin))

## Further Reading

* [A list of all F3D command-line options.](OPTIONS.md)
* [How to interact with scenes in F3D.](INTERACTIONS.MD)
* [How to play animations in F3D.](ANIMATIONS.md)
* [How to specify colormaps in F3D.](COLOR_MAPS.md)
* [Limitations and how to troubleshoot F3D.](LIMITATIONS_AND_TROUBLESHOOTING.md)
