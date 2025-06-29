# Classes Introduction

This is only an introduction to libf3d classes.
For the complete documentation, please consult the [libf3d doxygen documentation.](https://f3d.app/doc/libf3d/doxygen/).

## Engine class

The engine class is the main class that needs to be instantiated. All other classes instance are provided by the engine using getters, `getScene`, `getWindow`, `getInteractor`, `getOptions`.

The engine factory lets you choose between the different types of OpenGL rendering backend.
The generic `create()` is recommended in most cases and will use the best context possible available on your system.
However, it's possible to force the rendering backend in some specific use cases:

- `createGLX()`: force usage of GLX backend, works on Linux only and requires a X11 server to run.
- `createWGL()`: force usage of WGL native backend on Windows.
- `createEGL()`: force usage of EGL backend, recommended when doing offscreen rendering with a GPU available. Requires EGL library available. No interactor provided.
- `createOSMesa()`: force usage of OSMesa backend (software rendering), recommended when doing offscreen rendering without any GPU. Requires OSMesa library available. No interactor provided.
- `createNone()`: do not use any rendering. Useful to retrieve metadata only.
- `createExternal()`: the user is responsible of the rendering stack. It lets the user integrate libf3d in other frameworks like Qt or GLFW. No interactor provided. See [Context](#context-class) documentation for more info.
  An additional boolean argument is available to specify if offscreen rendering is requested when relevant on the selected rendering backend.

A static function `loadPlugin` can also be called to load reader plugins. It must be called before loading any file. An internal plugin containing VTK native readers can be loaded by calling `f3d::engine::loadPlugin("native");`. Other plugins maintained by F3D team are available if their build is enabled: `alembic`, `assimp`, `draco`, `hdf`, `occt` and `usd`.
If CMake option `F3D_PLUGINS_STATIC_BUILD` is enabled, the plugins listed above are also static just like `native` plugin.
All static plugins can be loaded using `f3d::engine::autoloadPlugins()`.

## Scene class

The scene class is responsible to `add` file from the disk into the scene. It supports reading multiple files at the same time and even mesh from memory.
It is possible to `clear` the scene and to check if the scene `supports` a file.

## Context class

Convenience class providing generic context API when using a external rendering backend (using `f3d::engine::createExternal()` factory).

## Window class

The window class is responsible for rendering the data.
Window lets you `render`, `renderToImage` and control other parameters of the window, like icon or windowName.

## Interactor class

When provided by the engine, the interactor class lets you choose how to interact with the data.

It contains the animation API to start and stop animation as well as recover the number of available animations and their names.

Interactor lets you add, remove and trigger your [commands](../user/COMMANDS.md).
The commands let you interact with the libf3d in a very flexible manner.

Interactor also lets you add and remove bindings in order to modify how
the libf3d react to different interactions, eg. when a key is pressed or when a file is dropped.
The libf3d has [default bindings](../user/INTERACTIONS.md) but this API lets you defined your own
by specifying [binds](../user/CONFIGURATION_FILE.md#Bind) and commands.
This API also lets you control the content of the cheatsheet.

Use `log::setVerboseLevel(log::VerboseLevel::DEBUG)` to print debug information on interaction and command use.

Of course, you can use `start` and `stop` to control the interactor behavior.
`start` lets you specify time for the event loop and a `std::function` to execute at each loop.

## Camera class

Provided by the window, this class lets you control the camera. You can either specify the camera position, target, and up direction directly, or specify movements relative to the current camera state.

## Image class

A generic image class that can either be created from a window, from an image filepath or even from a data buffer. It supports comparison making it very practical in testing context.

## Log class

A class to control logging in the libf3d. Simple using the different dedicated methods (`print`, `debug`, `info`, `warn`, `error`) and `setVerboseLevel`, you can easily control what to display. Please note that, on windows, a dedicated output window may be created.

## Options class

This class lets you control the behavior of the libf3d. An option is basically a value that can be a optional or not. There is different API to access it, see the exhaustive [doc](OPTIONS.md).

## Reader class

This class is used internally by the [plugin SDK](PLUGINS.md) and is not intended to be used directly.

## Plugin class

This class is used internally by the [plugin SDK](PLUGINS.md) and is not intended to be used directly.
