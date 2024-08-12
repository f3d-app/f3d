# Usage

Once F3D has been [installed](INSTALLATION.md), you should be able to open any [supported file](#supported-file-formats),
by either:
* Using F3D automatically, from your file manager, by directly opening a file.
* Running F3D and then dragging and dropping files into it to open them.
* By running F3D from the terminal with a set of command-line [options](OPTIONS.md).
* As a [thumbnailer](DESKTOP_INTEGRATION.md) for all supported file formats with certain file managers.

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

The **full scene** formats (.gltf/.glb, .3ds, .wrl, .obj, .fbx, .dae, .off, .x, .3mf, .usd) contain not only *geometry*,
but also some scene information like *lights*, *cameras*, *actors* in the scene, as well as *texture* properties.
By default, all this information will be loaded from the file and displayed.
For file formats that do not support it, **a default scene** is created.

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

F3D can play animations for a number of file formats (.ex2/.e/.exo/.g, .gltf/.glb, .fbx, .dae, .x, .usd) if the file contains an animation.
It is possible to select the animation to play using `--animation-index`, or to play all animations at once using `--animation-index=-1` (.gltf/.glb only).
When F3D plays an animation, it assumes the time unit is in seconds to show accurate speed of animation. Use `--animation-speed-factor` if
an adjustment is needed. By default, F3D will try update the scene 60 times per seconds, use `--animation-frame-rate` to change that if needed. Press "W" hotkey to cycle through available animations.

## Plugins

If you installed F3D using a package manager, it's possible that the packager chose to bundle the plugins in different packages or to list plugin dependencies as optional dependencies to reduce the reduce the number of dependencies of the main package.
In this case, in order to open a file that requires a plugin, you will have to make sure all needed dependencies are installed and specify which plugin you want to load in order to be able to open this file. You can either use the `--load-plugins` option or add a line in the [configuration file](CONFIGURATION_FILE.md), if not already. Several plugins can be specified by giving a comma-separated list.
Here is the list of plugins provided officially by F3D:

- **alembic**: ABC support
- **assimp**: FBX, DAE, OFF, DXF, X and 3MF support
- **draco**: DRC support
- **exodus**: EX2 support
- **occt**: STEP and IGES support
- **usd**: USD, USDA, UDSC and USDZ support
- **vdb**: VDB support (experimental)

> Note: If you downloaded the binaries from the Release page, it's not necessary to specify manually the plugins above, all of them are loaded automatically.

Here is how the plugins are searched (by precedence order):
1. Search the static plugins
2. Consider the option given is a full path
3. Search in the paths specified in `F3D_PLUGINS_PATH` environment variable
4. Search in a directory relative to the F3D application: `../lib`
5. Rely on OS specific paths (e.g. `LD_LIBRARY_PATH` on Linux or `DYLD_LIBRARY_PATH` on macOS)

You can also try plugins maintained by the community. If you have created a plugin and would like it to be listed here, please submit a pull request.

- **Abaqus**: ODB support by @YangShen398 ([repository](https://github.com/YangShen398/F3D-ODB-Reader-Plugin))
