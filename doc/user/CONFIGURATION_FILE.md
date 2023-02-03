# Configuration File

Almost all the command-line options can be controlled using a configuration file.
This configuration file uses the "long" version of the options in a JSON
formatted file to provide values for these options.

These options can be organized by block using a regular expression for each block
in order to provide different default values for the different filetypes.

A special block, named `global`, will apply to all files.
Using a command-line option will override the corresponding value in the config file.

The `global` block and command-line options are only taken into account on the first load
and not on subsequent loads, when switching between files.
The regular expression blocks are always taken into account, even when loading further files.

A typical config file may look like this:

```javascript
{
   "global": {
       "bg-color": "0.7,0.7,0.7",
       "color": "0.5,0.1,0.1",
       "fxaa": true,
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

F3D provides a default config file for generic usage (`config.json`) and a thumbnail specific config file (`thumbnail.json`).
You can edit these files or copy then into specific locations (see below) in order to customize F3D behavior.

The following command-line options <b> cannot <b> be set via config file:
`help`, `version`, `readers-list`, `config`, `dry-run`.

The following command-line options <b>can only</b> be set in the global block of the config file:
`no-render`, `inputs`, `output`, `quiet`, `verbose`, `resolution`, `position` and all testing options.

Boolean options that have been turned on in the configuration file can be turned
off on the command line if needed, eg: `--point-sprites=false`.

The configuration file possible locations depends on your operating system.
They are considered in the below order and only the first found will be used.

 * Linux: `${XDG_CONFIG_HOME}/.config/f3d/config.json`, `~/.config/f3d/config.json`, `/etc/f3d/config.json`, `/usr/share/f3d/config.json`, `[install_dir]/share/f3d/config.json`
 * Windows: `%APPDATA%\f3d\config.json`, `[install_dir]\config.json`
 * macOS: `${XDG_CONFIG_HOME}/.config/f3d/config.json`, `~/.config/f3d/config.json`, `/usr/local/etc/f3d/config.json`, `f3d.app/Contents/Resources/config.json`

The binary release will install the config files.
On Linux, they will be installed in `[install_dir]/share/f3d/`, on Windows, it will be installed in the install directory, on macOS, it will be installed in the bundle.

Please note there is a command line option to control the configuration file to read. Using it, one can specify an absolute/relative path for the configuration path, but also
only the filename or filestem (`.json` will be added) to look for in the locations listed above, instead of looking for `config.json`, eg: `f3d --config=custom_config` will look
for `custom_config.json` in locations listed above.
