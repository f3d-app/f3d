# Configuration File

## Options

Almost all the command-line [options](OPTIONS.md) can be controlled using configuration files.
Configuration files uses the "long" version of the command-line options in a JSON
formatted file to provide values for these options. It is also possible to use
the [libf3d options](../libf3d/OPTIONS.md) syntax.

These options can be organized by block using a regular expression, glob, or exact match
for each block in order to provide different default values for the different filetypes. For
more on glob and exact matching see the [Glob and Exact Matching section](#glob-and-exact-matching).

Using a command-line option will override similar option set in any config files.

Some options are only taken into account on the first load and not on subsequent loads,
when switching between files.

A typical config file with options may look like this:

```json
[
  {
    "options": {
      "background-color": "0.7,0.7,0.7",
      "color": "0.5,0.1,0.1",
      "anti-aliasing": true,
      "timer": true,
      "progress": true,
      "axis": true,
      "bar": true,
      "roughness": 0.2,
      "grid": true,
      "scalar-coloring": true
    }
  },
  {
    "match": ".*vt.",
    "options": {
      "edges": true
    }
  },
  {
    "match": ".*gl(tf|b)",
    "options": {
      "raytracing": true,
      "denoise": true,
      "samples": 3
    }
  },
  {
    "match": ".*mhd",
    "options": {
      "volume": true
    }
  }
]
```

Here, the first block defines a basic configuration with many desired options for all files.
The second block specifies that all files ending with vt., eg: vtk, vtp, vtu, ... will be shown with edges visibility turned on.
The third block specifies raytracing usage for .gltf and .glb files.
The last block specifies that volume rendering should be used with .mhd files.

The following options <b> cannot </b> be set via config file:
`help`, `version`, `list-readers`, `list-rendering-backends`, `scan-plugins`, `config`, `no-config`, `define`, `reset` and `input`.

The following options <b>are only taken on the first load</b>:
`no-render`, `output`, `position`, `resolution`, `frame-rate` and all testing options.

Boolean options that have been turned on in the configuration file can be turned
off on the command line if needed, eg: `--point-sprites=false`.

### Imperative Options

Command line options and options that are changed interactively overrides options that are set in configuration files.
This is not always a desired behavior, so in order to force an option to always be taken into account even if set in command line or changed interactively, it is possible to use imperative options, by adding a `!` in front of the option name, eg:

```json
[
  {
    "options": {
      "!axis": true
    }
  },
  {
    "match": ".*(stl)",
    "options": {
      "!edges": true
    }
  }
]
```

In the above example, when loading or reloading a file, the axis is always turned on and when loading a .stl file, the edges are always turned on.

## Bindings

All interaction bindings can be configured using configuration files.
The logic is the same as with options, where configuration blocks are used depending on the filename being loaded using regular expression.
A `bindings` block can be added to specify associations between binds (eg : `Ctrl+O`) and one or multiple commands.

A typical config file with bindings may look like this:

```json
[
  {
    "bindings": {
      "Ctrl+Shift+O": "toggle ui.filename",
      "Any+3": "roll_camera -90",
      "O": "set_camera isometric"
    }
  },
  {
    "match": ".*vtu",
    "bindings": {
      "Any+3": "roll_camera 90",
      "Shift+O": "toggle model.point_sprites.enable",
      "Ctrl+O": [
        "toggle render.grid.enable",
        "toggle scene.camera.orthographic"
      ],
      "None+I": "toggle ui.axis"
    }
  }
]
```

Here, the first block define new bindings for all and any files.
It even replace an existing default [interaction](INTERACTIONS.md) on the `O` key with its own.
Each bind is associated to the [command](COMMANDS.md) to execute when it is pressed.

In the second block, new bindings are defined for files ending in `.vtu`, and there bindings
will only be available when loading such a file.
In the first config block, we define bindings for all and any files. It also replace an existing default
interaction on the `Any+3` bind and even define a bindings that have multiple commands using a json array
on the `Ctrl+O` bind.

Please note this configuration feature is only available through config file and not through the command line.
However, it is possible to check your current binding configuration by using the `--list-bindings` CLI options.

### Bind

A bind is the association of a modifier and an interaction.

Supported modifiers are:

`None` : No modifiers is pressed, can be omitted completely.
`Ctrl` : Control key is pressed, no distinction between left and right.
`Shift` : Shift key is pressed, no distinction between left and right.
`Ctrl+Shift` : Control and Shift keys are pressed, no distinction between left and right.
`Any` : A special modifier that does not consider modifiers keys but can only be reached
if there is no bind with the same interaction.

Supported interactions are legion, eg:

- Keyboard letter keys, eg:
  - `O`
  - `P`
- Keyboard number keys, eg:
  - `2`
  - `5`
- Keyboard special keys, eg:
  - `Enter`
  - `Delete`
- Other interactions, eg:
  - `Drop` when files or directories are dropped on F3D window

To identify the interaction to use, just use `f3d --verbose` and perform the interaction, F3D will log it like this:

```
Interaction: KeyPress Ctrl+Shift+Insert
```

Please note that the interaction itself can be modified by the modifiers, hence some binds may be unreachable depending on your
keyboard layout, eg. on a `QWERTY` layout, the bind `Exclam` is unreachable while `Shift+Exclam` is reachable.
We tried to make the default bindings of F3D to be a keyboard layout independent as possible, but feel free to redefined them
for your needs.

## Glob and Exact Matching

While regex is the default match type, glob and exact match types are also supported. Below is an example using glob and exact matching.

```json
[
  {
    "options": {
      "background-color": "0.7,0.7,0.7",
      "color": "0.5,0.1,0.1",
      "anti-aliasing": true,
      "timer": true,
      "progress": true,
      "axis": true,
      "bar": true,
      "roughness": 0.2,
      "grid": true,
      "scalar-coloring": true
    }
  },
  {
    "match-type": "glob",
    "match": "*vt?",
    "options": {
      "edges": true
    }
  },
  {
    "match-type": "glob",
    "match": "**/*gl{tf,b}",
    "options": {
      "raytracing": true,
      "denoise": true,
      "samples": 3
    }
  },
  {
    "match-type": "exact",
    "match": "/path/to/file.mhd",
    "options": {
      "volume": true
    }
  }
]
```

For glob patterns, globstar (`**`) is supported.

Glob and exact matching can be used for bindings as well.

## Locations

Single .json file for configuration is supported, as well a config directory containing multiple .json config files.
In the case of config directory, .json file are read in alphabetical order and an option or binding in a block can be overridden
by the same option or bind being set in another block in another .json file read after.

F3D provides a default config directory for generic usage (`config.d`) and a thumbnail specific config directory (`thumbnail.d`).
You can edit the file contained in these directory or add your own in specific directories (see below).

F3D looks for configuration files in different locations depending on your operating system.
Existing configuration files are read in order and combined with later entries, potentially overriding previously read entry with the same names.

- Linux: `/etc/f3d/config(.json,.d)`, `/usr/share/f3d/configs/config(.json,.d)`, `[install_dir]/share/f3d/configs/config(.json,.d)`, `${XDG_CONFIG_HOME}/f3d/config(.json,.d)`
- Windows: `[install_dir]\share\f3d\configs\(config.json,.d)`, `%APPDATA%\f3d\(config.json,.d)`
- macOS: `/usr/local/etc/f3d/config(.json,.d)`, `f3d.app/Contents/Resources/configs/config(.json,.d)`, `${HOME}/Library/Application Support/f3d/config(.json,.d)`

Please note that, on Linux, `XDG_CONFIG_HOME` implementation can fallback on `HOME` environment variables as specified [here](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html).

The binary release will install the default config directory.
On Linux, they will be installed in `[install_dir]/share/f3d/configs/`, on Windows, they will be installed in `[install_dir]\share\f3d\configs\`, on macOS, it will be installed in the bundle.

Please note there is a command line option to control the configuration file to read. Using it, one can specify an absolute/relative path for the configuration path, but also
only the filename or filestem (`.json` and `.d` will be added) to look for in the locations listed above, , eg: `f3d --config=custom_config` will look
for `custom_config.json` and `custom_config.d` in locations listed above.
When specifying an absolute/relative path for the configuration file, a single file is read. If not, all files from locations listed above, with the overriding logic specified above.
