# Build guide

F3D uses a CMake based build system, so building F3D just requires installing
needed dependencies, configuring and building. If you are not used to such processes
please take a look at our [getting started guide](GETTING_STARTED.md).

## Dependencies

* [CMake](https://cmake.org) >= 3.1.
* [VTK](https://vtk.org) >= 9.0.0 (9.2.6 recommended).
* A C++17 compiler.
* A CMake-compatible build system (Visual Studio, XCode, Ninja, Make, etc.).
* Optionally, [Assimp](https://www.assimp.org/) >= 5.0.
* Optionally, Open CASCADE [OCCT](https://dev.opencascade.org/) >= 7.5.2.
* Optionally, [Alembic](http://www.alembic.io/) >= 1.7.
* Optionally, [OpenUSD](https://openusd.org/release/index.html) >= 23.05.
* Optionally, [OpenVDB](https://www.openvdb.org/download/) >= 10.0.0, enable `IOOpenVDB` VTK module.
* Optionally, [Draco](https://google.github.io/draco/) >= 1.5.
* Optionally, [Python](https://www.python.org/) >= 3.6 and [pybind11](https://github.com/pybind/pybind11) >= 2.2.
* Optionally, [Java](https://www.java.com) >= 18.
* Optionally, [OpenEXR](https://openexr.com/en/latest/) >= 3.0.

## VTK compatibility

As stated in the dependencies, F3D is compatible with VTK >= 9.0.0, however, many features are only available in certain conditions. We suggest using VTK 9.2.6 with RenderingRayTracing, RenderingExternal, IOExodus and IOOpenVDB modules enabled in order to get as many features as possible in F3D.

## Configuration and building

Configure and generate the project with CMake,
then build the software using your build system.

Here is some CMake options of interest:
* `F3D_BUILD_APPLICATION`: Build the F3D executable.
* `BUILD_TESTING`: Enable the [tests](TESTING.md).
* `F3D_MACOS_BUNDLE`: On macOS, build a `.app` bundle.
* `F3D_WINDOWS_GUI`: On Windows, build a Win32 application (without console).
* `F3D_WINDOWS_BUILD_SHELL_THUMBNAILS_EXTENSION`: On Windows, build the shell thumbnails extension.
* `F3D_PLUGINS_STATIC_BUILD`: Build all plugins as static library (embedded into `libf3d`) and automatically loaded by the application. Incompatible with `F3D_MACOS_BUNDLE`.

Some modules, plugins and bindings depending on external libraries can be optionally enabled with the following CMake variables:

* `F3D_MODULE_RAYTRACING`: Support for raytracing rendering. Requires that VTK has been built with `OSPRay` and `VTK_MODULE_ENABLE_VTK_RenderingRayTracing` turned on. Disabled by default.
* `F3D_MODULE_EXTERNAL_RENDERING`: Support for external render window. Requires that VTK has been built with `VTK_MODULE_ENABLE_VTK_RenderingExternal` turned on. Disabled by default.
* `F3D_MODULE_EXR`: Support for OpenEXR images. Requires `OpenEXR`. Disabled by default.
* `F3D_PLUGIN_BUILD_EXODUS`: Support for ExodusII (.ex2) file format. Requires that VTK has been built with `IOExodus` module (and `hdf5`). Enabled by default.
* `F3D_PLUGIN_BUILD_OCCT`: Support for STEP, IGES and BREP file formats. Requires `OpenCASCADE`. Disabled by default.
* `F3D_PLUGIN_BUILD_ASSIMP`: Support for FBX, DAE, OFF, DXF, X and 3MF file formats. Requires `Assimp`. Disabled by default.
* `F3D_PLUGIN_BUILD_ALEMBIC`: Support for ABC file format. Requires `Alembic`. Disabled by default.
* `F3D_PLUGIN_BUILD_DRACO`: Support for DRC file format. Requires `Draco`. Disabled by default.
* `F3D_PLUGIN_BUILD_USD`: Support for USD file format. Requires `OpenUSD`. Disabled by default.
* `F3D_PLUGIN_BUILD_VDB`: Support for VDB file format. Requires that VTK has been built with `IOOpenVDB` module (and `OpenVDB`). Disabled by default.
* `F3D_BINDINGS_PYTHON`: Generate python bindings (requires `Python` and `pybind11`). Disabled by default.
* `F3D_BINDINGS_JAVA`: Generate java bindings (requires `Java` and `JNI`). Disabled by default.

## VCPKG

It's possible to use VCPKG to automatically build dependencies. A manifest file `vcpkg.json` is available at your convenience. Please read [VCPKG documentation](https://vcpkg.io/en/getting-started.html).  
Basically, just install VCPKG, and configure F3D using `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake` option.

## Installing

Installation can be done through CMake, by running the following command:

```
cmake --install ${your_build_dir}
```

Individual components can also be installed by specifying the component name:

```
cmake --install ${your_build_dir} --component ${component_name}
```

Here is the list of all the components:

Name|Installed by default|Operating system|Description
------|------|------|------
`application`|YES|ALL|F3D application
`configuration`|NO|ALL|Default configuration files, `config` and `thumbnail`.
`library`|YES|ALL|libf3d library binaries
`plugin`|YES|ALL|libf3d plugins
`dependencies`|NO|ALL|libf3d runtime dependencies. Can be used to create a self-contained and relocatable package. System libraries are excluded.
`sdk`|NO|ALL|libf3d SDK (headers and CMake config files)
`licenses`|YES|ALL|F3D and third party licenses
`documentation`|YES|Linux|`man` documentation
`shellext`|YES|Windows/Linux|Desktop integration
`python`|YES|ALL|Python bindings
`java`|YES|ALL|Java bindings
`mimetypes`|NO|Linux|Plugins mimetype XML files for integration with Freedesktop
`assets`|YES|Linux|Assets for integration with Freedesktop
