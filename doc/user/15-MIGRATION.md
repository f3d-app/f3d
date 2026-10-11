# Migrate from v3.5

This guide explains how to migrate F3D usage from v3.5 to v4.0.

## Application CLI options

`-s/--scalar-coloring` has been replaced by `-s/--coloring-mode`. `--coloring-mode` is a string taking `material`, `scivis` or `direct` value.  
If not defined, `material` is used. If defined but no value is specified, `scivis` is used.

`-y/--coloring-component` doesn't accept negative values anymore. Do not define if you want to use the magnitude, and use `--coloring-mode=direct` instead of `--coloring-component=-2`

`--coloring-by-cells` has been replaced by `--coloring-field`. `--coloring-field` is a string taking `any`, `points` or `cells` value.

`-u/--blur-background` has been removed. Use `--hdri-skybox=blurred` instead.

`--animation-progress` was a boolean toggling a progress bar during animation playback. It is now a string selecting the progress bar mode: `none` (hidden), `default` (the progress bar alone) or `advanced` (the progress bar with time range, animation name, current time labels and key frame markers). Replace `true` with `default` (or `advanced`) and `false` with `none`.

Refer to [CLI options](../03-OPTIONS.md) for more details.

## Application bindings

Some default bindings have changed or removed:

- <kbd>S</kbd> now cycles the coloring mode (`material`/`scivis`/`direct`) instead of cycling the arrays
- <kbd>Y</kbd> now cycles the arrays instead of cycling the components (<kbd>SHIFT</kbd>+<kbd>Y</kbd> can be used now to cycle the components)
- <kbd>C</kbd> has been removed because <kbd>Y</kbd> cycles all the arrays (points and cells)
- <kbd>D</kbd> (raytracing denoiser) has been removed, you can use the command `toggle render.raytracing.denoise`
- <kbd>B</kbd> (scalar bar) has been removed, you can use the command `toggle ui.bar`
- <kbd>I</kbd> (volume inversion) has been removed, you can use the command `toggle model.volume.inverse`
- <kbd>Z</kbd> (fps counter) has been removed, you can use the command `toggle ui.fps`
- <kbd>U</kbd> (skybox blur) has been removed if favor of the new skybox mode cycle binding <kbd>J</kbd>
- <kbd>CTRL</kbd>+<kbd>K</kbd> (notifications) has been removed, you can use the command `toggle ui.notifications.enable`
- <kbd>J</kbd> now cycles between skybox modes (enabled, disabled, blurred)
- <kbd>H</kbd> (cheatsheet) has been moved to <kbd>F1</kbd>
- <kbd>SHIFT</kbd>+<kbd>H</kbd> (scene hierarchy) has been moved to <kbd>H</kbd>
- <kbd>&uarrow</kbd> has been moved to <kbd>F5</kbd>
- <kbd>&darrow</kbd> has been moved to <kbd>CTRL</kbd>+<kbd>SHIFT</kbd>+<kbd>R</kbd>
- <kbd>&larrow</kbd> has been moved to <kbd>&uarrow</kbd>
- <kbd>&rarrow</kbd> has been moved to <kbd>&darrow</kbd>
- <kbd>CTRL</kbd>+<kbd>&larrow</kbd> has been moved to <kbd>CTRL</kbd>+<kbd>&uarrow</kbd>
- <kbd>CTRL</kbd>+<kbd>&rarrow</kbd> has been moved to <kbd>CTRL</kbd>+<kbd>&darrow</kbd>

## F3D_PLUGINS_PATH

When running F3D, it was possible to specify the path for loading plugins using the environment variable `F3D_PLUGINS_PATH`.
This variable has been removed in favor of the CLI option `--plugins-path` which is more secure.

## Commands

The following commands have been removed and should be replaced:

- `cycle_anti_aliasing` -> `cycle render.effect.antialiasing.mode`
- `cycle_blending` -> `cycle render.effect.blending.mode`
- `cycle_point_sprites` -> `cycle model.point_sprites.type`
- `increase_light_intensity` -> `increase render.light.intensity` (increments are different)
- `decrease_light_intensity` -> `decrease render.light.intensity` (increments are different)
- `increase_opacity` -> `increase model.color.opacity`
- `decrease_opacity` -> `decrease model.color.opacity`
- `cycle_interactor_style` -> `cycle interactor.style`
- `jump_to_keyframe*` -> ``jump_to_key_frame*`

The `jump_to_frame` and `jump_to_keyframe` commands no longer take a second boolean argument and now always perform an absolute jump. To perform a relative jump, use the new `jump_to_frame_relative` and `jump_to_key_frame_relative` commands:

- `jump_to_frame 10 false` -> `jump_to_frame 10`
- `jump_to_frame 1 true` -> `jump_to_frame_relative 1`
- `jump_to_keyframe 4 false` -> `jump_to_key_frame 4`
- `jump_to_keyframe 1 true` -> `jump_to_key_frame_relative 1`

## Packagers

### CMake variables change

`F3D_LINUX_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX` has been removed in favor of
`F3D_LINUX_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_SYSCONFDIR`, which default to `OFF`.
The default installation location for configuration files is now `/usr/share/f3d/configs`.

### Install components

These components are now installed by default:

- `configuration`
- `sdk`
- `plugin_sdk`
- `mimetypes`
- `colormaps`

See building documentation for more information.
