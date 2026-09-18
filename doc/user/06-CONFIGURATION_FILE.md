# Configuration file

## Options

You can control almost all [command-line options](03-OPTIONS.md) using configuration files.
Configuration files use the "long" version of the command-line options in a JSON
formatted file to provide values for these options. You can also use
the [libf3d options](../libf3d/03-OPTIONS.md) syntax.

Each configuration file is a JSON array of blocks. Each block can define `options`, `bindings`, and an optional `match` rule.
Use a regular expression, glob, or exact match for each block to provide different default values for different file formats.
For more on glob and exact matching see the [Glob and exact matching section](#glob-and-exact-matching).

A command-line option overrides similar options set in any config files.

Some options apply only on the first load and not on subsequent loads,
when switching between files.

A typical config file with options looks like this:

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
The second block shows all files ending with vt., eg: vtk, vtp, vtu, ... with edges visibility turned on.
The third block specifies raytracing usage for .gltf and .glb files.
The last block uses volume rendering with .mhd files.

The following options <b> cannot </b> be set via config file:
`help`, `version`, `list-readers`, `list-rendering-backends`, `scan-plugins`, `config`, `no-config`, `define`, `reset` and `input`.

The following options <b>are only taken on the first load</b>:
`no-render`, `output`, `position`, `resolution`, `frame-rate` and all testing options.

Turn off boolean options from the command line when needed, eg: `--point-sprites=false`.

### Imperative options

Command-line options and interactive changes override options that are set in configuration files.
When a configuration option must override command-line options and interactive changes, use an imperative option by adding `!` in front of the option name, eg:

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

In the above example, F3D always turns on the axis when loading or reloading a file. It also always turns on edges when loading a .stl file.

## Bindings

Configure all interaction bindings using configuration files.
The logic is the same as with options: configuration blocks apply depending on the filename being loaded using regular expression.
Add a `bindings` block to associate binds (e.g `Ctrl+O`) with one or multiple commands.

A typical config file with bindings looks like this:

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
      "Shift+O": "set model.point_sprites.type circle",
      "Ctrl+O": [
        "toggle render.grid.enable",
        "toggle scene.camera.orthographic"
      ],
      "None+I": "toggle ui.axis"
    }
  }
]
```

Here, the first block defines new bindings for all files.
It even replaces an existing default [interaction](04-INTERACTIONS.md) on the <kbd>O</kbd> key with its own.
Each bind associates to the [command](07-COMMANDS.md) to execute when it is pressed.

The second block defines new bindings for files ending in `.vtu`, and these bindings
are available only when loading such a file.
In the first config block, we define bindings for all files. It also replaces an existing default
interaction on the `Any+3` bind and defines a binding that has multiple commands using a json array
on the `Ctrl+O` bind.

This configuration feature is available only through config file and not through the command line.
Check your current binding configuration by using the `--list-bindings` CLI option.

### Bind

A bind is the association of a modifier and an interaction.

Supported modifiers are:

- `None` : No modifier is pressed. You can omit this modifier completely.
- `Ctrl` : Control key is pressed. F3D does not distinguish left and right keys.
- `Shift` : Shift key is pressed. F3D does not distinguish left and right keys.
- `Ctrl+Shift` : Control and Shift keys are pressed. F3D does not distinguish left and right keys.
- `Any` : A special modifier that does not consider modifiers keys but can only be reached
  if there is no bind with the same interaction.

On macOS, define bindings in configuration files using `Ctrl` and `Ctrl+Shift`. F3D maps them to `Cmd` and `Cmd+Shift` respectively. The Cheatsheet displays the correct binding combination regardless.

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

To identify the interaction to use, run `f3d --verbose` and perform the interaction. F3D logs it like this:

```text
Interaction: KeyPress Ctrl+Shift+Insert
```

The modifiers can change the interaction itself, so some binds may be unreachable depending on your
keyboard layout, eg. on a `QWERTY` layout, the bind `Exclam` is unreachable while `Shift+Exclam` is reachable.
F3D default bindings are as keyboard-layout independent as possible, but you can redefine them
for your needs.

## Glob and exact matching

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

Glob patterns support globstar (`**`).

Use glob and exact matching for bindings as well.

## Locations

F3D supports a single .json configuration file, as well as a config directory containing multiple .json config files.
In a config directory, F3D reads .json files in alphabetical order. The same option or bind in a later block can override an earlier one.

F3D provides a default config directory for generic usage (`config.d`) and a thumbnail specific config directory (`thumbnail.d`).
You can edit the files contained in these directories or add your own in specific directories (see below).

F3D looks for configuration files in different locations depending on your operating system.
F3D reads existing configuration files in order and combines them with later entries potentially overriding previously read entries with the same names.
For thumbnails, replace `config` by `thumbnail`, as the `f3d` command receives the thumbnails configuration using the `--config` [CLI option](03-OPTIONS.md).

- Linux: `/etc/f3d/config(.json,.d)`, `/usr/share/f3d/configs/config(.json,.d)`, `[install_dir]/share/f3d/configs/config(.json,.d)`, `${XDG_CONFIG_HOME}/f3d/config(.json,.d)`
- Windows: `[install_dir]\share\f3d\configs\(config.json,.d)`, `%APPDATA%\f3d\(config.json,.d)`
- macOS: `/usr/local/etc/f3d/config(.json,.d)`, `f3d.app/Contents/Resources/configs/config(.json,.d)`, `${HOME}/Library/Application Support/f3d/config(.json,.d)`

On Linux, `XDG_CONFIG_HOME` implementation can fallback on `HOME` environment variables as specified by the [XDG Base Directory Specification](https://specifications.freedesktop.org/basedir/latest/).

The binary release installs the default config directory.
On Linux, it installs files in `[install_dir]/share/f3d/configs/`. On Windows, it installs files in `[install_dir]\share\f3d\configs\`. On macOS, it installs files in the bundle.

Use the command-line option to control the configuration file to read. Specify an absolute or relative path for the configuration path, or
only the filename or filestem (F3D adds `.json` and `.d`) to look for in the locations listed above, , eg: `f3d --config=custom_config` looks
for `custom_config.json` and `custom_config.d` in locations listed above.
When you specify an absolute or relative path for the configuration file, F3D reads a single file. Otherwise, F3D reads all files from the locations listed above, with the overriding logic specified above.

To check which config file is found and used, check the verbose output, eg. for thumbnails config: `f3d --config=thumbnail --verbose` (or `f3d-console.exe --config=thumbnail --verbose` on Windows) :

```text
========== Initializing Options ==========
Found available config path
Candidate config file not found: "/etc/f3d/thumbnail.json"
Candidate config file not found: "/etc/f3d/thumbnail.d"
Candidate config file not found: "/usr/share/f3d/configs/thumbnail.json"
Candidate config file not found: "/usr/share/f3d/configs/thumbnail.d"
Candidate config file not found: "/home/glow/dev/f3d/pack/F3D-3.5.0-Linux-x86_64-raytracing/share/f3d/configs/thumbnail.json"
Config file found: "/home/glow/dev/f3d/pack/F3D-3.5.0-Linux-x86_64-raytracing/share/f3d/configs/thumbnail.d"
Candidate config file not found: "/home/glow/.config/f3d/thumbnail.json"
Candidate config file not found: "/home/glow/.config/f3d/thumbnail.d"
Using config directory /home/glow/dev/f3d/pack/F3D-3.5.0-Linux-x86_64-raytracing/share/f3d/configs/thumbnail.d
```
