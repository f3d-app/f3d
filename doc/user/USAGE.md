# Quickstart Guide

This guide will help you get started using F3D.

As an overview, you'll learn how to run F3D and open supported files. You'll then learn how to configure basic scene constructions, animations and plugins.

## Prerequisites

To use F3D, you'll need to [install](INSTALLATION.md) F3D. See the [Installation](INSTALLATION.md) page for the latest release package supported by your system.

## Run F3D

Once F3D has been [installed](INSTALLATION.md), you should be able to open a [supported file](#supported-file-formats) in any of the following ways:

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

| Name | File Extension(s) | Type of Scene(s) Supported |
| -- | -- | -- |
| Legacy VTK | **.vtk** | Default |
| VTK XML | **.vt[p\|u\|r\|i\|s\|m]** | Default |
| Polygon File Format | **.ply** | Default |
| Standard Triangle Language | **.stl** | Default |  
| DICOM | **.dcm** | Default |
| NRRD ("nearly raw raster data") | **.nrrd/.nhrd** | Default |
| MetaHeader MetaIO | **.mhd/mha** | Default |
| Tag Image File Format (TIFF) 2D/3D | **.tif/.tiff** | Default |
| EXODUS II | **.e/.ex2/.exo/.g** | Default |
| CityGML | **.gml** | Default |
| Point Cloud | **.pts** | Default |
| Standard for the Exchange of Product Data (STEP) | **.step/.stp** | Default |
| Initial Graphics Exchange Specification (IGES) | **.iges/.igs** | Default |
| Open CASCADE Technology (OCCT) BRep format | **.brep** | Default |
| Alembic | **.abc** | Default |
| Wavefront OBJ | **.obj** | Default, Full |
| GL Transmission Format | **.gltf/.glb** | Default, Full |
| Autodesk 3D Studio | **.3ds** | Full |
| Virtual Reality Modeling Language (VRML) | **.wrl** | Full |
| Autodesk Filmbox (FBX) | **.fbx** | Full |
| COLLADA | **.dae** | Full |
| Object File Format | **.off** | Full |
| Drawing Exchange Format | **.dxf** | Full |
| DirectX | **.x** | Full |
| 3D Manufacturing Format | **.3mf** | Full |
| Universal Scene Description | **.usd** | Full |
| \***VDB** (Experimental) | **.vdb** | Default |

\***Note:** Support for VDB is experimental.

## Scene construction

For most file formats, F3D constructs **a default scene**, loading only the *geometry* from the file.

However, formats supporting **full scenes** will automatically load additional scene properties including *lights*, *cameras*, *actors* and *texture*.

To disable these properties, you can specify the `--geometry-only` [option](OPTIONS.md) from the command line to use a default scene construction instead.

Please refer to the table of supported file formats to see what kind of scenes your file supports.

For **default scene** formats, certain default values are set automatically:
 - texture-*: ""
 - line-width: 1.0
 - point-size: 10.0
 - opacity: 1.0
 - color: 1.0, 1.0, 1.0
 - emissive-factor: 1.0, 1.0, 1.0
 - normal-scale: 1.0
 - metallic: 0.0
 - roughness: 0.3

They will be overridden when using corresponding [options](OPTIONS.md).

## Animations

F3D can play animations for a number of file formats if the file contains an animation. See the table of [supported file formats](#supported-file-formats) for supported file formats.

To select the animation to play, you can use the `--animation-index` option. To play all animations at once, use `--animation-index=-1` (.gltf/.glb only).

F3D uses the second as the default unit of time. To adjust the speed factor of your animations, use the `--animation-speed-factor` option. 

Additionally, F3D uses a default frame rate of 60 frames per second. To change the frame rate of your animations, use `--animation-frame-rate`. 

Press the "W" hotkey to cycle through available animations.

## Plugins

If you installed F3D using a package manager, it's possible that the packager chose to bundle the plugins in different packages or to list plugin dependencies as optional in order to reduce the reduce the number of dependencies of the main package.

In order to open a file that requires a plugin, make sure you have installed all necessary dependencies. You then need to specify which plugin you want to load in order to be able to open this file. You can either use the `--load-plugins` option or add it in the [configuration file](CONFIGURATION_FILE.md) if not already specified. You can specify multiple plugins in a single comma-separated list.

### Supported plugins

F3D officially provides the following plugins and their supported file formats:

- **alembic**: ABC support
- **assimp**: FBX, DAE, OFF, DXF, X and 3MF support
- **draco**: DRC support
- **exodus**: EX2 support
- **occt**: STEP and IGES support
- **usd**: USD, USDA, UDSC and USDZ support
- **vdb**: VDB support (experimental)

> Note: If you downloaded the binaries from the Release page, it's not necessary to specify manually the plugins above, all of them are loaded automatically.

Here is how the plugins are searched (in preceding order):
1. Search the static plugins.
2. Consider the option given is a full path.
3. Search in the paths specified in `F3D_PLUGINS_PATH` environment variable.
4. Search in a directory relative to the F3D application: `../lib`.
5. Rely on OS specific paths (e.g. `LD_LIBRARY_PATH` on Linux or `DYLD_LIBRARY_PATH` on macOS).

You can also try plugins maintained by the community. If you have created a plugin and would like it to be listed here, please submit a pull request.

- **Abaqus**: ODB support by @YangShen398 ([repository](https://github.com/YangShen398/F3D-ODB-Reader-Plugin))
