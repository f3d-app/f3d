# Examples

This folder contains examples on how to use `libf3d`. All the examples are using `find_package(f3d)` so they are intended to be built against an existing f3d installation.
The code is documented in a didactic way to help with understanding them.

## libf3d

Examples usages of the libf3d, which requires the `F3D_INSTALL_SDK` cmake option when building F3D.

## plugins

Examples plugins for the libf3d, which requires the `F3D_INSTALL_PLUGIN_SDK` cmake option when building F3D.

## Building

Each example is intended to be built as a separate CMake project, however top level `CMakeLists.txt` are provided if needed.

```
cmake -Df3d_DIR=/path/to/f3d/install/lib/cmake/f3d /path/to/example/dir
cmake --build
```

Then run the example
