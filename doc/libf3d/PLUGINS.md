# Plugin SDK

When calling `find_package(f3d)` in CMake, a few macros are made available to you to generate a plugin which allow you to extend libf3d to support your own file format.
> Please consider [contributing](../dev/CONTRIBUTE.md) your plugin in [F3D directly](https://github.com/f3d-app/f3d/tree/master/plugins) if you think it can be useful to the community.  
> You can also consider contributing directly [in VTK](https://gitlab.kitware.com/vtk/vtk/blob/master/Documentation/dev/git/develop.md).

## Creating a plugin

You can take a look at the example in the [examples/plugin](https://github.com/f3d-app/f3d/tree/master/examples/plugin) directory or at the official [plugins](https://github.com/f3d-app/f3d/tree/master/plugins).

The first thing (and most difficult part) you have to do is creating a VTK reader (or a VTK importer if you want to support a full scene with materials, lights and cameras), and wrap it into a VTK module. You can create several readers in the same VTK module if you need to support several file formats in a single plugin.

Then, declare the reader(s) and the plugin using the CMake macros:

```cmake
f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME "ReaderName"
  EXTENSIONS ${reader_ext}     # set the extensions the reader can support
  MIMETYPES ${reader_mime}     # set the mimetypes the reader can support
  VTK_READER ${vtk_classname}  # set the name of the VTK class you have created
  DESCRIPTION "Reader description"
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

## Loading your plugin

The plugin can be loaded using `f3d::engine::loadPlugin("path or name")` api if you are using libf3d, or `--load-plugins="path or name"` option if you are using F3D application.
