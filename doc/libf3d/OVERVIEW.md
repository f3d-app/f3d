# libf3d - A library to render 3D meshes

By Michael Migliore and Mathieu Westphal.

libf3d is a BSD-licensed C++17 library to open and render 3D meshes. It is of course used by F3D.
libf3d API is simple and easy to learn. Python bindings are provided through pybind11. Java bindings are also available.
libf3d API is still in alpha version and may change drastically in the future.

## Getting Started

Rendering a file and starting the interaction is very easy:

```cpp
#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/scene.h>

// Load VTK native readers
f3d::engine::autoloadPlugins();

// Create a f3d::engine
f3d::engine eng = f3d::engine::create();

// Add a file into a scene
eng.getScene().add("path/to/file.ext");

// Start rendering and interacting
eng.getInteractor().start();
```

As well as loading multiple files:

```cpp
#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/scene.h>

// Load VTK native readers
f3d::engine::autoloadPlugins();

// Create a f3d::engine
f3d::engine eng = f3d::engine::create();

// Load multiples geometries
eng.getScene().add({"path/to/file.ext", "path/to/file2.ext"});

// Start rendering and interacting
eng.getInteractor().start();
```

It's also possible to load a geometry from memory buffers:

```cpp
#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/scene.h>

// Create a f3d::engine
f3d::engine eng = f3d::engine::create();

// Create a single triangle
f3d::mesh_t mesh = {};
mesh.points = { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f };
mesh.face_sides = { 3 };
mesh.face_indices = { 0, 1, 2 };
eng.getScene().add(mesh);

// Start rendering and interacting
eng.getInteractor().start();
```

Manipulating the window directly can be done this way:

```cpp
#include <f3d/engine.h>
#include <f3d/image.h>
#include <f3d/scene.h>
#include <f3d/window.h>

// Load VTK native readers
f3d::engine::autoloadPlugins();

// Create a f3d::engine with a offscreen window
f3d::engine eng = f3d::engine::create(true);

// Load a geometry
eng.getScene().add("path/to/file.ext");

// Set the window size and render to an image
f3d::image img = eng.getWindow().setSize(300, 300).renderToImage();

// Save the image to a file
img.save("/path/to/img.png");
```

Changing some options can be done this way:

```cpp
#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/options.h>
#include <f3d/scene.h>

// Load VTK native readers
f3d::engine::autoloadPlugins();

// Create a f3d::engine
f3d::engine eng = f3d::engine::create();

// Recover the options and set the wanted value
f3d::options& opt = eng.getOptions();
opt.render.effect.ambient_occlusion = true;
opt.render.effect.antialiasing.enable = true;

// Standard libf3d usage
eng.getScene().add("path/to/file.ext");
eng.getInteractor().start();
```

Most options are dynamic, some are only taken into account when loading a file. See the [options](OPTIONS.md) documentation.

Find more examples in the [examples directory](https://github.com/f3d-app/f3d/tree/master/examples),
you can also find other usages in the [testing directory](https://github.com/f3d-app/f3d/tree/master/library/testing).

For the complete documentation, please consult the [libf3d doxygen documentation.](https://f3d.app/doc/libf3d/doxygen/).

## Building against the libf3d

Please follow instructions in the [F3D build guide](../dev/BUILD.md), make sure to build with `BUILD_SHARED_LIBS=ON` and to install the `sdk` component then use CMake to find the libf3d
and link against it like this in your CMakeLists.txt:

```cmake
find_package(f3d REQUIRED COMPONENTS library)
[...]
target_link_libraries(target f3d::libf3d)
```

## find_package COMPONENTS

When using `find_package(f3d)` and `sdk` CMake component have been installed, you can require specific components:

- `application`: access to f3d::f3d target
- `library`: access to f3d::libf3d target and include dir
- `plugin_sdk`: access to CMake macros to create plugins, the f3d::vtkext target and include dir

see the installed `f3dConfig.cmake` file for more info.

## Extending libf3d with plugins

In order to add new supported file format to libf3d, you can create a plugin using CMake macros. Please follow instructions in the [F3D plugin SDK guide](PLUGINS.md).
Also make sure the `plugin_sdk` component have been installed as well as the `sdk` component.
