# Class Listing

## Engine class

The engine class is the main class that needs to be instantiated. All other classes instance are provided by the engine using getters, `getLoader`, `getWindow`, `getInteractor`, `getOptions`.

The engine constructor lets you choose the type of window in its constructor, `NONE`, `NATIVE`, `NATIVE_OFFSCREEN`, `EXTERNAL`. Default is `NATIVE`. See [Window class](window-class) documentation for more info. Please note that the engine will not provide a interactor with `NONE` and `EXTERNAL`.

## Loader class

The loader class is responsible to read and load the file from the disk. It supports reading multiple files and even folders.

## Window class

The window class is responsible for rendering the meshes. It supports multiple modes.

* `NONE`: A window that will not render anything, very practical when only trying to recover meta-information about the data.

* `NATIVE`: Default mode where a window is shown onscreen using native graphical capabilities.

* `NATIVE_OFFSCREEN`: Use native graphical capabilities for rendering, but unto an offscreen window, which will not appear on screen, practical when generating screenshots.

* `EXTERNAL`: A window where the OpenGL context is not created but assumed to have been created externally. To be used with other frameworks like Qt or GLFW.

Window lets you `render`, `renderToImage` and control other parameters of the window, like icon or windowName.

## Interactor class

When provided by the engine, the interactor class lets you choose how to interact with the data.

It contains the animation API to start and stop animation.

Interactor also lets you set your interaction callbacks in order to modify how the interaction with the data is done.

Of course, you can use `start` and `stop` to control the interactor behavior.

## Camera class

Provided by the window, this class lets you control the camera. You can either specify the camera position, target, and up direction directly, or specify movements relative to the current camera state.

## Image class

A generic image class that can either be created from a window, from an image filepath or even from a data buffer. It supports comparison making it very practical in testing context.

## Log class

A class to control logging in the libf3d. Simple using the different dedicated methods (`print`, `debug`, `info`, `warn`, `error`) and `setVerboseLevel`, you can easily control what to display. Please note that, on windows, a dedicated output window may be created.

## Options class

This class lets you control the behavior of the libf3d. An option is basically a string used as a key associated with a value, see the exhaustive [list](OPTIONS.md).
