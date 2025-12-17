# Examples

This folder contains examples on how to use `libf3d`. All the examples are using some form of `find_package(f3d)` so they are intended to be built against an existing f3d installation with the `sdk` or `plugin_sdk` component installed.
The code is documented in a didactic way to help with understanding them.

## libf3d

Examples usages of the libf3d, which requires the `sdk` cmake component to be installed.

## plugins

Examples plugins for the libf3d, which also requires the `plugin_sdk` cmake component to be installed.

## Building

Each example is intended to be built as a separate CMake project, however there are top level `CMakeLists.txt` in each directory for easier usage.

eg:

```
cmake -Df3d_DIR=/path/to/f3d/install/lib/cmake/f3d /path/to/example/dir/libf3d
cmake --build .
```

Then run the example individually.
