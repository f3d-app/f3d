# Migrate from v3.5

This guide explains how to migrate the libf3d code base from v3.5 to v4.0.

> [!WARNING]
> This guide assumes all deprecation warnings have been addressed, since the deprecated APIs have been removed.

## User callback

When calling `f3d::interactor::start()` or `f3d::interactor::playInteraction()`, it was possible to set a user callback automatically called at each event loop.
If you were setting such callback, please call `f3d::interactor::setEventLoopUserCallback()` manually before calling `start()` or `playInteractor()`.
The callback takes an argument of type `f3d::interactor::interactor_state_t` allowing you to retrieve useful information about the interactor state.

## F3D_PLUGINS_PATH

When running F3D, it was possible to specify the path for loading plugins using the environment variable `F3D_PLUGINS_PATH`. This variable has been removed in favor of the CLI option `--plugins-path` which is more secure.

## scene.supports method
`scene::supports()` method signature changed, it now returns `f3d::file_availability` enum instead of `bool`. Here is how you can check if a file is supported now:
```cpp
if (scene.supports("some.obj") == f3d::file_availability::SUPPORTED)
```