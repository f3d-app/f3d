# Migrate from v3.5

This guide explains how to migrate the libf3d code base from v3.5 to v4.0.

> [!WARNING]
> This guide assumes all deprecation warnings have been addressed, since the deprecated APIs have been removed.

## Library options

### Enable options

Many `enable` libf3d options have been removed in favor of extending possible values on the `mode`/`type` related libf3d options.

`render.effect.blending.enable` have been removed in favor of setting `render.effect.blending.mode` to `none`, which is its new default.
So to enable blending, just set the mode to the value that used to be the default, `ddp`.

`render.effect.antialiasing.enable` have been removed in favor of setting `render.effect.antialiasing.mode` to `none`, which is its new default.
So to enable anti-aliasing, just set the mode to the value that used to be the default, `fxaa`.

`model.point_sprites.enable` have been removed in favor of setting `model.point_sprites.type` to `none`, which is its new default.
So to enable point sprites, just set the type to the value that used to be the default, `sphere`.

`ui.scene_hierarchy` have been removed in favor of `ui.scene_hierarchy.enable`.

`render.background.skybox` is now a enum domain with valid values `disabled` (default), `enabled` and `blurred`.
`render.background.blur.enable` has been removed (use `render.background.skybox` option to `blurred`).
`render.background.blur.coc` have been replaced by `render.background.blur_coc`.

The `ui.animation_progress` option (CLI `--animation-progress`) was a boolean toggling a progress bar during animation playback. It is now a string selecting the progress bar mode: `none` (hidden), `default` (the progress bar alone) or `advanced` (the progress bar with time range, animation name, current time labels and key frame markers). Replace `true` with `default` (or `advanced`) and `false` with `none`.

### Coloring mode

Scivis options have been revamped entirely.

`model.scivis.enable` has been removed in favor of the new `model.coloring` option.
So to enable scivis, just set `model.coloring` to the value `scivis`.

`model.scivis.cells` has been removed in favor of the new `model.scivis.field` option.
By default, all arrays are active but if you need cell arrays only, just set `model.scivis.field` to `cells`.

`model.scivis.component` is now optional and doesn't accept negative values anymore.
If you need coloring by magnitude (old value `-1`), just leave this option empty, that's the default.
If you need direct coloring (old value `-2`), set `model.coloring` to the value `direct`.

`model.coloring` must be set to `scivis` or `direct` if `model.volume.enable` is true.

## User callback

When calling `f3d::interactor::start()` or `f3d::interactor::playInteraction()`, it was possible to set a user callback automatically called at each event loop.
If you were setting such callback, please call `f3d::interactor::setEventLoopUserCallback()` manually before calling `start()` or `playInteractor()`.
The callback takes an argument of type `f3d::interactor::interactor_state_t` allowing you to retrieve useful information about the interactor state.

## Context symbol

The function `f3d::context::getSymbol` function is now expecting a library full path or a filename.
The library prefix and extension is not appended automatically anymore.

## DPI scaling

`f3d::window::getDPIScale()` should now be used instead of the static `f3d::utils::getDPIScale()` API to get the DPI scaling value.

## Add Binding API

The `f3d::interactor::addBinding` API has been refactored to take a struct with all parameters to reduce complexity calling the API. The arguments themselves have not been changed.

## Bindings

### Python

The following Python setter methods have been removed in favor of new properties, which can also be read.

- `window.set_position(x, y)` -> `window.position = (x, y)`
- `engine.set_cache_path(path)` -> `engine.cache_path = path`

### WebAssembly

The following WebAssembly methods have been removed in favor of new properties, which can also be read.

- `engine.setCachePath(path)` -> `engine.cachePath = path`

### C

Some functions has been renamed for consistency. All functions allocating an object have the word `create` and all functions deleting an object have the word `destroy`.

| 3.5             | 4.0              |
| --------------- | ---------------- |
| `f3d_*_new*`    | `f3d_*_create*`  |
| `f3d_*_free*`   | `f3d_*_destroy*` |
| `f3d_*_delete*` | `f3d_*_destroy*` |

## scene.supports method

`scene::supports()` method signature changed, it now returns `f3d::file_availability` enum instead of `bool`. Here is how you can check if a file is supported now:

```cpp
if (scene.supports("some.obj") == f3d::file_availability::SUPPORTED)
```

Other languages API behavior changed accordingly:

- C API:
  - `f3d_scene_supports()` used to return 1 if the file was supported and 0 otherwise. It now returns an int: 0 if supported, 1 for unsupported extension, 2 for unsupported content, -1 if the scene or file path is NULL.
  - `f3d_interactor_add_binding` now requires a struct parameter over a list of arguments. This includes a `repeat` member, which specifies that the binding is repeatedly applied when holding down the key.
- Java API:
  - `Scene.supports()` used to return a boolean. It now returns the `Scene.FileAvailability` enum and throws `IllegalArgumentException` if the file path is null.
  - `Interactor.addBinding()` now requires a class instance parameter over a list of arguments. This includes a `repeat` argument which specifies that the binding is repeatedly applied when holding down the key.
- Python API:
  - `scene.supports()` used to return bool. Now returns f3d.FileAvailability.
  - `interactor.add_binding()` now requires keyword arguments. This includes a `repeat` argument which specifies that the binding is repeatedly applied when holding down the key. `command` must now be passed as an array.
- Webassembly API: `scene.supports()` used to return bool. Now returns enum FileAvailability.

## scene animation API

The whole scene animation API is now available in a new dedicated `f3d::animation` class.

First recover the `animation` instance (always available) from the scene: `scene::getAnimation()`, then use the following:

- `scene::loadAnimationTime(double timeValue) ` -> `animation::loadTime(timeValue)`
- `std::pair<double, double> scene::animationTimeRange()` -> `std::pair<double, double> animation::getTimeRange()`
- `std::vector<double> scene::getAnimationKeyFrames()` -> `std::vector<double> animation::getKeyFrames()`
- `unsigned int scene::availableAnimations() const` -> `unsigned int animation::count() const`
- `std::string scene::getAnimationName(int index = -1)` -> `std::string animation::getName(std::optional<int> index = std::nullopt)`
- `std::vector<std::string> scene::getAnimationNames()` -> `std::vector<std::string> animation::getNames()`
- `toggleAnimation(AnimationDirection direction)` -> `toggle(Direction direction)`
- `startAnimation(AnimationDirection direction)` -> `start(Direction direction)`
- `stopAnimation()` -> `stop()`
- `bool isPlayingAnimation()` -> `isPlaying()`
- `AnimationDirection getAnimationDirection()` -> `Direction getDirection()`

All bindings have been updated accordingly.

## Plugin developers

If you were using the VTK extension module with your plugin, headers are now located in `f3d/vtk_ext/` folder instead of `f3d/` directly.
