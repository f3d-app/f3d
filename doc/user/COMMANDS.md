# Commands

F3D provides access to commands through [interactive console](#interactive-console), [command script](#command-script---command-script) and [bindings configuration](CONFIGURATION_FILE.md#bindings).
Commands let you trigger specific behavior that may not be available otherwise.
Please note commands are currently experimental and the behaviors, actions may be added or removed without deprecation.
Action names and arguments may also change without deprecation.

Commands have the following syntax:

`action [args]`

## libf3d provided commands

The libf3d provides a few commands, many related to manipulating libf3d (options)[../libf3d/OPTIONS.md].

`set option.name values`: A command to set a libf3d option, eg: `set scene.up.direction +Z` or `set render.hdri.file "/path/to/file with spaces.png"`

`toggle option.name`: A command to toggle a boolean libf3d option, eg: `toggle ui.scalar_bar`.

`reset option.name`: A command to reset a libf3d option to its default values, eg: `reset render.background.blur.coc`.

`clear`: A command to clear console. No argument.

`print option.name`: A command to print the value of an libf3d option, eg: `print scene.up.direction`.

`set_reader_option Reader.option_name value`: A specific command to set a [reader option](SUPPORTED_FORMATS.md#reader-options), eg: `set_reader_option QuakeMDL.skin_index 1`

`cycle_anti_aliasing`: A specific command to cycle between the anti-aliasing method (`none`,`fxaa`,`ssaa`).

`cycle_animation`: A specific command to cycle `scene.animation.index` option using model information, No argument.

`cycle_coloring field/array/component`: A specific command to manipulate scivis options using model information.
Supports `field`, `array` or `component` arguments, see [documentation](INTERACTIONS.md#cycling-coloring).
eg: `cycle_coloring array`.

`roll_camera value`: A specific command to roll the camera on its side, takes an angle in degrees as an argument.
eg: `roll_camera 120`.

`elevation_camera value`: A specific command to tilt the camera up or down, takes an angle in degrees as an argument.
eg: `elevation_camera 120`.

`azimuth_camera value`: A specific command to tilt the camera right or left, takes an angle in degrees as an argument.
eg: `azimuth_camera 120`.

`increase_light_intensity`: A specific command to increase light intensity. No argument.

`decrease_light_intensity`: A specific command to decrease light intensity. No argument.

`increase_opacity`: A specific command to increase opacity. Unset opacity will be treated as if it has a value of 1. No argument.

`decrease_opacity`: A specific command to decrease opacity. Unset opacity will be treated as if it has a value of 1. No argument.

`print_scene_info`: A specific command to print information about the scene, No argument.

`print_coloring_info`: A specific command to print information about coloring settings, No argument.

`print_mesh_info`: A specific command to print information from the mesh importer, No argument.

`print_options_info`: A specific command to print libf3d options that have a value, No argument.

`set_camera front/top/right/isometric`: A specific command to position the camera in the specified location relative to the model.
Supports `front`, `top`, `right`, `isometric` arguments. eg: `set_camera top`.

`toggle_volume_rendering`: A specific command to toggle `model.volume.enable` and print coloring information. No argument.

`stop_interactor`: A specific command to stop the interactor hence quitting the application. No argument.

`reset_camera`: A specific command to reset the camera to its original location. No argument.

`toggle_animation`: A specific command to start/stop the animation. No argument.

`cycle_verbose_level` : A specific command to cycle between the verbose levels (`Debug`, `Info`, `Warning`, `Error`, `Quiet`).

`add_files [path/to/file1] [path/to/file2]`: A specific command to add files to the scene (overridden by a F3D specific command, see below). Take one or more files as arguments.
eg: `add_files /path/to/dragon.vtu`.

`alias [alias_name] [command]`: A specific command to create an alias for a command. Take an alias name and a command as arguments.
eg: `alias myrotate roll_camera 90`.

`help [command]: A specific command to print help about a specific command. eg: `help set_camera`.

## F3D provided specific commands

The F3D application provides a few more commands.

`exit`: A specific command to quit the application. No argument.

`load_previous_file_group [keep_camera]`: A specific command to load the previous file or file group. Camera state is kept if `keep_camera` is true (default: false).

`load_next_file_group [keep_camera]`: A specific command to load the next file or file group. No argument. Camera state is kept if `keep_camera` is true (default: false).

`reload_current_file_group`: A specific command to reload the current file or file group. No argument.

`add_current_directories`: A specific command to add all files from the current file or file group directories. No argument.

`take_screenshot [filename]`: A specific command to [take a screenshot](INTERACTIONS.md#taking-screenshots). If filename is not specified,
rely on the `--screenshot-filename` CLI option. eg: `take_screenshot path/to/file.png`.

`take_minimal_screenshot [filename]`: A specific command to [take a minimal screenshot](INTERACTIONS.md#taking-screenshots). If filename is not specified,
rely on the `--screenshot-filename` CLI option. eg: `take_screenshot path/to/file.png`.

`add_files [path/to/file1] [path/to/file2]`: A specific command to add files to the scene according to the current grouping logic. Take one or more files as arguments.
eg: `add_files /path/to/dragon.vtu`.

`set_hdri [path/to/hdri]`: A specific command to set and use an HDRI image. Take a HDRI file as an argument.
eg: `set_hdri /path/to/file.hdr`.

`add_files_or_set_hdri [path/to/file1] [path/to/file2]`: A specific command that will process each files and either, `set_hdri` if the provided file uses a recognised HDR extension or `add_files` otherwise. Take one or more files as arguments.
eg: `add_files_or_set_hdri /path/to/dragon.vtu /path/to/file.hdr`.

`remove_current_file_group`: A specific command to remove current file group and load the next file group if any. No argument.

`remove_file_groups`: A specific command to remove all files. No argument.

`open_file_dialog`: A specific command to open a file dialog to selected a file to load. No argument.

## Command Script (`--command-script`)

F3D provides a feature to execute commands from a script file using the `--command-script` [CLI option](OPTIONS.md). This allows users to automate a sequence of commands by listing them in a plain text file, eg: `f3d --command-script path/to/command_script.txt`.

Example Command Script, commands are separated by new lines, comments are supported:

```shell
# A comment
roll_camera 90
toggle ui.scalar_bar
print_scene_info # Another comment
increase_light_intensity
```

## Interactive Console

If F3D is built with `F3D_MODULE_UI` support, pressing <kbd>Esc</kbd> will open the console. It's possible to type any command in the input field and pressing <kbd>Enter</kbd> will trigger the command instantly.

Press <kbd>Tab</kbd> to autocomplete the command and display suggestions.

Press <kbd>&uarr;</kbd> and <kbd>&darr;</kbd> to navigate through the command history.

Press <kbd>Esc</kbd> to close the console.

## Command syntax

Command syntax is similar to bash, as in they will be split by "token" to be processed.

- Tokens are spaces separated, eg: `set scene.up.direction +Z`.
- Tokens can also be quoted to support spaces inside, eg: `set render.hdri.file "/path/to/file with spaces.png"`.
- Supported quotes are `` `'" ``, eg: `set render.hdri.file '/path/to/file with spaces.png'`.
- Quotes inside quotes are supported as well, eg: `set render.hdri.file "/path/to/file'with'quotes.png"`.
- Quotes and spaces can be escaped, eg: `set render.hdri.file /path/to/file\ with\ spaces\ and\ \'quotes\".png`.
- Comment are supported using `#`, Any character after will be ignored. Use `\#` to add it verbatim.
- Escapes can be escaped too: eg: `set render.hdri.file C:\\path\\to\\windows\\file.png`.
- Other escaped character will be processed as if the escape was not present, eg: `set scene.up.direction +\Z`
- Unfinished quoted section is invalid, eg: `set scene.up.direction "+Z`
- A escape at the end is also invalid, eg: `set scene.up.direction +Z\`
- Options values are [parsed](PARSING.md) according to their types.
