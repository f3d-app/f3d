# Migrate from v3.5

This guide explains how to migrate the libf3d code base from v3.5 to v4.0.

> [!WARNING]
> This guide assumes all deprecation warnings have been addressed, since the deprecated APIs have been removed.

## Enable options

Many `enable` libf3d options have been removed in favor of extending possible values on the `mode`/`type` related libf3d options.

`render.effect.blending.enable` have been removed in favor of setting `render.effect.blending.mode` to `none`, which is its new default.
So to enable blending, just set the mode to the value that used to be the default, `ddp`.

`render.effect.antialiasing.enable` have been removed in favor of setting `render.effect.antialiasing.mode` to `none`, which is its new default.
So to enable anti-aliasing, just set the mode to the value that used to be the default, `fxaa`.

`model.point_sprites.enable` have been removed in favor of setting `model.point_sprites.type` to `none`, which is its new default.
So to enable point sprites, just set the type to the value that used to be the default, `sphere`.

## User callback

When calling `f3d::interactor::start()` or `f3d::interactor::playInteraction()`, it was possible to set a user callback automatically called at each event loop.
If you were setting such callback, please call `f3d::interactor::setEventLoopUserCallback()` manually before calling `start()` or `playInteractor()`.
The callback takes an argument of type `f3d::interactor::interactor_state_t` allowing you to retrieve useful information about the interactor state.

## F3D_PLUGINS_PATH

When running F3D, it was possible to specify the path for loading plugins using the environment variable `F3D_PLUGINS_PATH`. This variable has been removed in favor of the CLI option `--plugins-path` which is more secure.
