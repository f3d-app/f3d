# Usage

Once F3D has been [installed](INSTALLATION.md), you should be able to open any [supported file](#supported-file-formats),
by either:
* Using F3D automatically, from your file manager, by directly opening a file.
* Running F3D and then dragging and dropping files into it to open them.
* By running F3D from the terminal with a set of command-line [options](OPTIONS.md).
* As a [thumbnailer](DESKTOP_INTEGRATION.md) for all supported file formats with certain file managers.

## Supported file formats

Here is the list of supported file formats:

* **.vtk** : the legacy VTK format
* **.vt[p\|u\|r\|i\|s\|m]** : XML based VTK formats
* **.ply** : Polygon File format
* **.stl** : Standard Triangle Language format
* **.dcm** : DICOM file format
* **.nrrd/.nhrd** : "nearly raw raster data" file format
* **.mhd/.mha** : MetaHeader MetaIO file format
* **.tif/.tiff** : TIFF 2D/3D file format
* **.ex2/.e/.exo/.g** : Exodus 2 file format
* **.gml** : CityGML file format
* **.pts** : Point Cloud file format
* **.step/.stp** : CAD STEP exchange ISO format
* **.iges/.igs** : CAD Initial Graphics Exchange Specification format
* **.brep** : Open CASCADE BRep format
* **.abc** : Alembic format
* **.vdb** : VDB format (experimental)
* **.obj** : Wavefront OBJ file format (full scene and default scene)
* **.gltf/.glb** : GL Transmission Format (full scene and default scene)
* **.3ds** : Autodesk 3D Studio file format (full scene)
* **.wrl** : VRML file format (full scene)
* **.fbx** : Autodesk Filmbox (full scene)
* **.dae** : COLLADA (full scene)
* **.off** : Object File Format (full scene)
* **.dxf** : Drawing Exchange Format (full scene)
* **.x** : DirectX Format (full scene)
* **.3mf** : 3D Manufacturing Format (full scene)

## Scene construction

The **full scene** formats (.gltf/.glb, .3ds, .wrl, .obj, .fbx, .dae, .off, .x, .3mf) contain not only *geometry*, 
but also some scene information like *lights*, *cameras*, *actors* in the scene, as well as *texture* properties.
By default, all this information will be loaded from the file and displayed. Use the `--geometry-only` [options](OPTIONS.md)
to modify this behavior. For file formats that do not support it, **a default scene** is created.

## Animations

F3D can play animations for a number of file formats (.ex2/.e/.exo/.g, .gltf/.glb, .fbx, .dae, .x) if the file contains an animation.
It is possible to select the animation to play using `--animation-index`, or to play all animations at once using `--animation-index=-1` (.gltf/.glb only).
When F3D play an animation, it assumes the time unit is in seconds to show accurate speed of animation. Use `--animation-speed-factor` if
an adjustment is needed. By default, F3D will try update the scene 60 times per seconds, use `--animation-frame-rate` to change that if needed.

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
