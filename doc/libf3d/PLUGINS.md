# Plugin SDK

Please follow instructions in the [F3D build guide](../dev/BUILD.md), make sure to build with `BUILD_SHARED_LIBS=ON` and to install the `plugin_sdk` component.
You will then be able to call `find_package(f3d REQUIRED COMPONENTS pluginsdk)` in your plugin CMakeLists.txt, a few macros are made available to you to generate a plugin which allow you to extend libf3d to support your own file format. Access to a f3d::vtkext VTK module is also provided if needed.

> Please consider [contributing](../../CONTRIBUTING.md) your plugin in [F3D directly](https://github.com/f3d-app/f3d/tree/master/plugins) if you think it can be useful to the community.
> You can also consider contributing directly [in VTK](https://gitlab.kitware.com/vtk/vtk/blob/master/Documentation/dev/git/develop.md).

## Creating a plugin

You can take a look at the example in the [examples/plugin](https://github.com/f3d-app/f3d/tree/master/examples/plugins) directory or at the official [plugins](https://github.com/f3d-app/f3d/tree/master/plugins).

The first thing (and most difficult part) you have to do is creating a VTK reader (or a VTK importer if you want to support a full scene with materials, lights and cameras), and wrap it into a VTK module. You can create several readers/importers in the same VTK module if you need to support several file formats in a single plugin.

Then, declare the reader(s) and the plugin using the CMake macros:

```cmake
f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME "ReaderName"
  EXTENSIONS "myext"                # set the extensions the reader can support
  MIMETYPES "application/vnd.myext" # set the mimetypes the reader can support
  VTK_READER ${vtk_classname}       # set the name of the VTK reader class you have created
  FORMAT_DESCRIPTION "description"  # set the proper name of the file format
  EXCLUDE_FROM_THUMBNAILER          # add this flag if you don't want thumbnail generation for this reader
  OPTIONS "option1" "option2"       # use this to define reader specific option that can be defined by the user
)

f3d_plugin_declare_reader(
  NAME "ReaderNameWithScene"
  EXTENSIONS "myext2"                # set the extensions the reader can support
  MIMETYPES "application/vnd.myext2" # set the mimetypes the reader can support
  VTK_IMPORTER ${vtk_classname}      # set the name of the VTK importer class you have created
  FORMAT_DESCRIPTION "description"   # set the proper name of the file format
  CUSTOM_CODE "file.inl"             # set this to add a custom code when instancing your class, this is where reader options should be processed
)

# More f3d_plugin_declare_reader calls are possible

f3d_plugin_build(
  NAME "PluginName"
  VERSION 1.0
  DESCRIPTION "Plugin description"
  VTK_MODULES CommonCore CommonExecutionModel # Add more VTK modules if necessary
)
```

If the build succeeds, a library called `libf3d-plugin-<name>.so` will be created (`f3d-plugin-<name>.dll` on Windows)
A JSON file of the following form will also be generated. It's used by F3D internally to get information about supported file formats.

```json
{
  "description": "Plugin description",
  "name": "PluginName",
  "readers": [
    {
      "description": "Reader description",
      "extensions": ["myext"],
      "mimetypes": ["application/vnd.myext"],
      "name": "ReaderName"
    }
  ],
  "type": "MODULE",
  "version": "1.0"
}
```

The list of existing mimetypes can be find [here](https://www.iana.org/assignments/media-types/media-types.xhtml). If your file format is not listed, the mimetype should be `application/vnd.${extension}`

## Loading your plugin

The plugin can be loaded using `f3d::engine::loadPlugin("path or name")` API if you are using libf3d, or `--load-plugins="path or name"` option if you are using F3D application.
The option can also be set in a configuration file that you could distribute with your plugin.

## f3d::vtkext

F3D provides access to a VTK modules containing utilities that may be useful for plugin developers:

- `vtkF3DFaceVaryingPointDispatcher`: A VTK filter that manipulates point data so that F3D can display them as face-varying data (used by `usd` plugin)
- `vtkF3DBitonicSort`: A VTK class that perform Bitonic Sort algorithm on the GPU (used by the translucent point sprites rendering algorithm)
- `vtkF3DImporter`: An Importer class that abstract away support for different version of VTK after some API changes.
- `vtkF3DGLTFImporter`: An custom glTF importer class that support armatures, useful when creating other plugin supporting glTF extensions.

For the complete documentation, please consult the [vtkext doxygen documentation.](https://f3d.app/doc/libf3d/vtkext_doxygen/).
