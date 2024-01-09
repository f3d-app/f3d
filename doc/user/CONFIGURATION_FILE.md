# Configuration File

Almost all the command-line options can be controlled using configuration files.
Configuration files uses the "long" version of the command-line options in a JSON
formatted file to provide values for these options.

These options can be organized by block using a regular expression for each block
in order to provide different default values for the different filetypes.

A special block, named `global`, will apply to all files.
Using a command-line option will override the corresponding value in the config file.

The `global` block and command-line options are only taken into account on the first load
and not on subsequent loads, when switching between files.
The regular expression blocks are always taken into account, even when loading further files.
Please note the regular expressions are not case-sensitive.

A typical config file may look like this:

```javascript
{
   "global": {
       "bg-color": "0.7,0.7,0.7",
       "color": "0.5,0.1,0.1",
       "anti-aliasing": true,
       "timer": true,
       "progress": true,
       "axis": true,
       "bar": true,
       "roughness": 0.2,
       "grid": true,
       "scalars": true
   },
   ".*vt.": {
       "edges": true
   },
   ".*gl(tf|b)": {
       "raytracing": true,
       "denoise": true,
       "samples": 3
   },
   ".*mhd": {
       "volume": true
   }
}
```
Here, the first block defines a basic global configuration with many desired options for all files.
The second block specifies that all files ending with vt., eg: vtk, vtp, vtu, ... will be shown with edges visibility turned on.
The third block specifies raytracing usage for .gltf and .glb files.
The last block specifies that volume rendering should be used with .mhd files.

Single .json file for configuration is supported, as well a config directory containing multiple .json config files.
In the case of config directory, .json file are read in alphabetical order and an option in a block can be overridden
by the same option being set in the same block in another .json file read after.

F3D provides a default config directory for generic usage (`config.d`) and a thumbnail specific config directory (`thumbnail.d`).
You can edit the file contained in these directory or copy then into specific locations (see below) in order to customize F3D behavior.

The following command-line options <b> cannot </b> be set via config file:
`help`, `version`, `readers-list`, `config`, `dry-run`.

The following command-line options <b>can only</b> be set in the global block of the config file:
`no-render`, `inputs`, `output`, `verbose`, `resolution`, `position` and all testing options.

Boolean options that have been turned on in the configuration file can be turned
off on the command line if needed, eg: `--point-sprites=false`.

The configuration file or directory possible locations depends on your operating system.
They are considered in the below order and only the first found will be used.

 * Linux: `${XDG_CONFIG_HOME}/.config/f3d/config(.json,.d)`, `~/.config/f3d/config(.json,d)`, `/etc/f3d/config(.json,.d)`, `/usr/share/f3d/configs/config(.json,.d)`, `[install_dir]/share/f3d/configs/config(.json,.d)`
 * Windows: `%APPDATA%\f3d\(config.json,.d)`, `[install_dir]\share\f3d\configs\(config.json,.d)`
 * macOS: `${XDG_CONFIG_HOME}/.config/f3d/config(.json,.d)`, `~/.config/f3d/config(.json,.d)`, `/usr/local/etc/f3d/config(.json,.d)`, `f3d.app/Contents/Resources/configs/config(.json,.d)`

The binary release will install the default config directory.
On Linux, they will be installed in `[install_dir]/share/f3d/configs/`, on Windows, they will be installed in `[install_dir]\share\f3d\configs\`, on macOS, it will be installed in the bundle.

Please note there is a command line option to control the configuration file to read. Using it, one can specify an absolute/relative path for the configuration path, but also
only the filename or filestem (`.json` and `.d`  will be added) to look for in the locations listed above, , eg: `f3d --config=custom_config` will look
for `custom_config.json` and `custom_config.d` in locations listed above.

Please note <b>only one</b> config file or directory will be found and parsed.
