# Commands

F3D provides access to commands through bindings configuration. They may be more ways to access them in the future.
Commands let you trigger specific behavior that may not be available otherwise.
Please note commands are currently experimental and the behaviors, actions may be added or removed without deprecation.
Action names and arguments may also change without deprecation.

Commands have the following syntax:

`action [args]`

## libf3d provided commands

The libf3d provides a few commands, many related to manipulating libf3d (options)[../libf3d/OPTIONS.md].

`set option.name values`: A command to set a libf3d option, eg: `set scene.up.direction +Z` or `set render.hdri.file "/path/to/file with spaces.png"`

`toggle option.name`: A command to toggle a boolean libf3d option, eg: `toggle ui.scalar_bar`. 

`reset option.name`: A command to reset a libf3d option to its default values, eg: `reset render.background.blur_coc`.

`print option.name`: A command to print the value of an libf3d option, eg: `print scene.up.direction`.

`cycle_animation`:  A specific command to cycle `scene.animation.index` option using model information, No argument.

`cycle_coloring field/array/component`:  A specific command to manipulate scivis options using model information. 
Supports `field`, `array` or `component` arguments, see [documentation](INTERACTIONS.md#cycling-coloring).
eg: `cycle_coloring array`.

`roll_camera value`: A specific command to roll the camera on its side, takes an angle in degrees as an argument. 
eg: `roll_camera 120`.

`increase_light_intensity`: A specific command to increase light intensity. No argument.

`decrease_light_intensity`: A specific command to decrease light intensity. No argument.

`print_scene_info`: A specific command to print information about the scene, No argument.

`set_camera front/top/right/isometric`: A specific command to position the camera in the specified location relative to the model.
Supports `front`, `top`, `right`, `isometric` arguments. eg: `set_camera top`.

`toggle_volume_rendering`: A specific command to toggle `model.volume.enable` and print coloring information. No argument.

`stop_interactor`: A specific command to stop the interactor hence quitting the application. No argument.

`reset_camera`: A specific command to reset the camera to its original location. No argument.

`toggle_animation`: A specific command to start/stop the animation. No argument.

`add_files [path/to/file1] [path/to/file2]`: A specific command to add files to the scene (overridden by a F3D specific command, see below). Take one or more files as arguments.
eg: `add_files /path/to/dragon.vtu`.

## F3D provided specific commands

The F3D application provides a few more commands.

`load_previous_file_group`: A specific command to load the previous file or file group. No argument.

`load_next_file_group`: A specific command to load the next file or file group. No argument.

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

## Command syntax

Command syntax is similar to bash, as in they will be split by "token" to be processed.

 - Tokens are spaces separated, eg: `set scene.up.direction +Z`.
 - Tokens can also be quoted to support spaces inside, eg:  `set render.hdri.file "/path/to/file with spaces.png"`.
 - Supported quotes are `` `'" ``, eg: `set render.hdri.file '/path/to/file with spaces.png'`.
 - Quotes inside quotes are supported as well, eg: `set render.hdri.file "/path/to/file'with'quotes.png"`.
 - Quotes and spaces can be escaped, eg: `set render.hdri.file /path/to/file\ with\ spaces\ and\ \'quotes\".png`.
 - Comment are supported using `#`, Any character after will be ignored. Use `\#` to add it verbatim.
 - Escapes can be escaped too: eg: `set render.hdri.file C:\\path\\to\\windows\\file.png`.
 - Other escaped character will be processed as if the escape was not present, eg: `set scene.up.direction +\Z`
 - Unfinished quoted section is invalid, eg: `set scene.up.direction "+Z`
 - A escape at the end is also invalid, eg: `set scene.up.direction +Z\`
