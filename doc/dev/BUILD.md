# Build guide

> [!NOTE]
> For WebAssembly cross-compilation, follow the dedicated guide [here](./BUILD_WASM.md).

F3D uses a CMake based build system, so building F3D just requires installing
needed dependencies, configuring and building. If you are not used to such processes
please take a look at our [getting started guide](GETTING_STARTED.md).

## Dependencies

- [CMake](https://cmake.org) >= 3.1.
- [VTK](https://vtk.org) >= 9.2.6 (9.5.0 recommended).
- A C++17 compiler.
- A CMake-compatible build system (Visual Studio, XCode, Ninja, Make, etc.).
- Optionally, [Assimp](https://www.assimp.org/) >= 5.4.0 (6.0.2 recommended).
- Optionally, Open CASCADE [OCCT](https://dev.opencascade.org/) >= 7.6.3 (7.9.1 recommended).
- Optionally, [Alembic](http://www.alembic.io/) >= 1.8.5.
- Optionally, [OpenUSD](https://openusd.org/release/index.html) >= 24.08 (25.05.01 recommended).
- Optionally, [OpenVDB](https://www.openvdb.org/download/) >= 12.0.0, enable `IOOpenVDB` module during VTK configuration.
- Optionally, [OSPray](https://www.ospray.org/) == v2.12.0, enable `RenderingRayTracing` module during VTK configuration.
- Optionally, [Draco](https://google.github.io/draco/) >= 1.5.6.
- Optionally, [Python](https://www.python.org/) >= 3.9 and [pybind11](https://github.com/pybind/pybind11) >= 2.9.2.
- Optionally, [Java](https://www.java.com) >= 17.
- Optionally, [OpenEXR](https://openexr.com/en/latest/) >= 3.0.1.

F3D is tested continuously against versions recommended by the [VFX reference platform](https://vfxplatform.com) defined for **CY2025**

## VTK compatibility

As stated in the dependencies, F3D is compatible with VTK >= 9.2.6, however, some features may not be available. We suggest using VTK 9.5.0 with RenderingRayTracing, IOExodus, IOHDF, IONetCDF and IOOpenVDB modules enabled in order to get as many features as possible in F3D.

## Configuration and building

Configure and generate the project with CMake,
then build the software using your build system.

Here is some CMake options of interest:

- `F3D_BUILD_APPLICATION`: Build the F3D executable.
- `BUILD_TESTING`: Enable the [tests](TESTING.md).
- `F3D_MACOS_BUNDLE`: On macOS, build a `.app` bundle.
- `F3D_WINDOWS_BUILD_SHELL_THUMBNAILS_EXTENSION`: On Windows, build the shell thumbnails extension.
- `F3D_WINDOWS_BUILD_CONSOLE_APPLICATION`: On Windows, build a supplemental Win32 console application.
- `F3D_PLUGINS_STATIC_BUILD`: Build all plugins as static library (embedded into `libf3d`) and automatically loaded by the application. Incompatible with `F3D_MACOS_BUNDLE`.
- `BUILD_SHARED_LIBS`: Build the libf3d and all plugins as static library (embedded into `f3d` executable). The `library` and `plugin_sdk` component will not be installed.

Some modules, plugins and language bindings depending on external libraries can be optionally enabled with the following CMake variables:

- `F3D_MODULE_RAYTRACING`: Support for raytracing rendering. Requires that VTK has been built with `OSPRay` and `RenderingRayTracing` turned on. Disabled by default.
- `F3D_MODULE_EXR`: Support for OpenEXR images. Requires `OpenEXR`. Disabled by default.
- `F3D_MODULE_UI`: Support for ImGui widgets. Uses provided ImGui. Enabled by default.
- `F3D_PLUGIN_BUILD_HDF`: Support for VTKHDF (.vtkhdf), ExodusII (.ex2), and NetCDF (.nc) file formats. Requires that VTK has been built with `IOHDF`, `IOExodus`, and `IONetCDF` modules (and `hdf5`). Enabled by default.
- `F3D_PLUGIN_BUILD_OCCT`: Support for STEP, IGES, BREP, and XBF file formats. Requires `OpenCASCADE`. Disabled by default.
- `F3D_PLUGIN_BUILD_ASSIMP`: Support for FBX, DAE, OFF, DXF, X and 3MF file formats. Requires `Assimp`. Disabled by default.
- `F3D_PLUGIN_BUILD_ALEMBIC`: Support for ABC file format. Requires `Alembic`. Disabled by default.
- `F3D_PLUGIN_BUILD_DRACO`: Support for DRC file format. Requires `Draco`. Disabled by default.
- `F3D_PLUGIN_BUILD_USD`: Support for USD file format. Requires `OpenUSD`. Disabled by default.
- `F3D_PLUGIN_BUILD_VDB`: Support for VDB file format. Requires that VTK has been built with `IOOpenVDB` module (and `OpenVDB`). Disabled by default.
- `F3D_BINDINGS_PYTHON`: Generate python bindings (requires `Python` and `pybind11`). Disabled by default.
- `F3D_BINDINGS_PYTHON_GENERATE_STUBS`: Generate python stubs (requires `Python` and `pybind11_stubgen`). Disabled by default.
- `F3D_BINDINGS_JAVA`: Generate java bindings (requires `Java` and `JNI`). Disabled by default.

Some dependencies are provided internally, eg: ImGui, dmon and others. Use `F3D_USE_EXTERNAL_*` to use an external version of these libraries.

## VCPKG

It's possible to use VCPKG to automatically build dependencies. A manifest file `vcpkg.json` is available at your convenience. Please read [VCPKG documentation](https://vcpkg.io/en/getting-started.html).  
Basically, just install VCPKG, and configure F3D using `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake` option.  
Only VTK is added to the manifest file and additional dependencies must be added manually in this file.

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

| Name            | Installed by default | Operating system | Description                                                                                                                 |
| --------------- | -------------------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------- |
| `application`   | YES                  | ALL              | F3D application.                                                                                                            |
| `configuration` | NO                   | ALL              | Default configuration files, `config` and `thumbnail`.                                                                      |
| `library`       | YES                  | ALL              | libf3d library binaries.                                                                                                    |
| `plugin`        | YES                  | ALL              | libf3d plugins.                                                                                                             |
| `dependencies`  | NO                   | ALL              | libf3d runtime dependencies. Can be used to create a self-contained and relocatable package. System libraries are excluded. |
| `sdk`           | NO                   | ALL              | libf3d SDK (headers and CMake config files) for `library` and `application` find_package components.                        |
| `plugin_sdk`    | NO                   | ALL              | libf3d plugin SDK (headers and CMake config files including macros) for `pluginsdk` find_package components.                |
| `licenses`      | YES                  | ALL              | F3D and third party licenses.                                                                                               |
| `documentation` | YES                  | Linux            | `man` documentation.                                                                                                        |
| `shellext`      | YES                  | Windows/Linux    | Desktop integration.                                                                                                        |
| `python`        | YES                  | ALL              | Python bindings.                                                                                                            |
| `java`          | YES                  | ALL              | Java bindings.                                                                                                              |
| `mimetypes`     | NO                   | Linux            | Plugins mimetype XML files for integration with Freedesktop.                                                                |
| `assets`        | YES                  | Linux            | Assets for integration with Freedesktop.                                                                                    |
| `colormaps`     | NO                   | ALL              | Color maps presets, see [documentation](../user/COLOR_MAPS.md)                                                              |
