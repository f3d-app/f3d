# libf3d - A library to render 3D meshes

By Michael Migliore and Mathieu Westphal.

libf3d is a BSD-licensed C++ library to open and render 3D meshes. It is of course used by F3D.
libf3d API is simple and easy to learn. Python bindings are provided through pybind11. Java bindings are also available.
libf3d API is still in alpha version and may change drastically in the future.

## Getting Started

Rendering a file and starting the interaction is very easy:

```cpp
#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/loader.h>

// Load VTK native readers
f3d::engine::autoloadPlugins();

// Create a f3d::engine
f3d::engine eng();

// Add a file to the loader and load it
eng.getLoader().addFile("path/to/file.ext").loadFile();

// Start rendering and interacting
eng.getInteractor().start();
```

Manipulating the window directly can be done this way:

```cpp
#include <f3d/engine.h>
#include <f3d/image.h>
#include <f3d/loader.h>
#include <f3d/window.h>

// Load VTK native readers
f3d::engine::autoloadPlugins();

// Create a f3d::engine
f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);

// Add a file to the loader and load it
eng.getLoader().addFile("path/to/file.ext").loadFile();

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
#include <f3d/loader.h>

// Load VTK native readers
f3d::engine::autoloadPlugins();

// Create a f3d::engine
f3d::engine eng();

// Recover the options and set the wanted value
eng.getOptions()
  .set("render.effect.ssao", true)
  .set("render.effect.fxaa", true);

// Standard libf3d usage
eng.getLoader().addFile("path/to/file.ext").loadFile();
eng.getInteractor().start();
```
Most options are dynamic, some are only taken into account when loading a file. See the [options](OPTIONS.md) documentation.

For more advanced usage, please take a look in the [testing directory](https://github.com/f3d-app/f3d/tree/master/library/testing).

For the complete documentation, please consult the [libf3d doxygen documentation.](https://f3d.app/doc/libf3d/doxygen/).

## Building against the libf3d

Please follow instructions in the [F3D build guide](../dev/BUILD.md), then use CMake to find the libf3d
and link against it like this in your CMakeLists.txt:

```cmake
find_package(f3d REQUIRED)
[...]
target_link_libraries(target f3d::libf3d)
```

## Extending libf3d with plugins

In order to add new supported file format to libf3d, you can create a plugin using CMake macros. Please follow instructions in the [F3D plugin SDK guide](PLUGINS.md).
