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
* **.abc** : Alembic format
* **.obj** : Wavefront OBJ file format (full scene and default scene)
* **.gltf/.glb** : GL Transmission Format (full scene and default scene)
* **.3ds** : Autodesk 3D Studio file format (full scene)
* **.wrl** : VRML file format (full scene)
* **.fbx** : Autodesk Filmbox (full scene)
* **.dae** : COLLADA (full scene)
* **.off** : Object File Format (full scene)
* **.dxf** : Drawing Exchange Format (full scene)

## Scene construction

The **full scene** formats (.gltf/.glb, .3ds, .wrl, .obj, .fbx, .dae, .off) contain not only *geometry*, 
but also some scene information like *lights*, *cameras*, *actors* in the scene, as well as *texture* properties.
By default, all this information will be loaded from the file and displayed. Use the `--geometry-only` [options](OPTIONS.md)
to modify this behavior. For file formats that do not support it, **a default scene** is created.

## Plugins

If you installed F3D using a package manager, it's possible that the packager chose to bundle the plugins in different packages or to list plugin dependencies as optional dependencies to reduce the reduce the number of dependencies of the main package.
In this case, in order to open a file that requires a plugin, you will have to make sure all needed dependencies are installed and specify which plugin you want to load in order to be able to open this file. You can either use the `--load-plugins` option or add a line in the [configuration file](CONFIGURATION_FILE.md), if not already.
Here is the list of plugins provided officially by F3D:

- **assimp**: FBX, DAE, OFF and DXF support
- **occt**: STEP and IGES support
- **alembic**: ABC support
- **exodus**: EX2 support

> Note: If you downloaded the binaries from the Release page, it's not necessary to specify manually the plugins above, all of them are loaded automatically.
