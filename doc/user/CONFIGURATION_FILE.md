# Configuration File

Almost all the command-line [options](OPTIONS.md) can be controlled using configuration files.
Configuration files uses the "long" version of the command-line options in a JSON
formatted file to provide values for these options. It is also possible to use
the [libf3d options](../libf3d/OPTIONS.md) syntax.

These options can be organized by block using a regular expression for each block
in order to provide different default values for the different filetypes.

Using a command-line option will override similar option set in any config files.

Some options are only taken into account on the first load and not on subsequent loads, 
when switching between files.

A typical config file may look like this:

```javascript
{
   ".*": {
       "bg-color": "0.7,0.7,0.7",
       "color": "0.5,0.1,0.1",
       "anti-aliasing": true,
       "timer": true,
       "progress": true,
       "axis": true,
       "bar": true,
       "roughness": 0.2,
       "grid": true,
       "scalar-coloring": true
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
Here, the first block defines a basic configuration with many desired options for all files.
The second block specifies that all files ending with vt., eg: vtk, vtp, vtu, ... will be shown with edges visibility turned on.
The third block specifies raytracing usage for .gltf and .glb files.
The last block specifies that volume rendering should be used with .mhd files.

Single .json file for configuration is supported, as well a config directory containing multiple .json config files.
In the case of config directory, .json file are read in alphabetical order and an option in a block can be overridden
by the same option being set in another block in another .json file read after.

F3D provides a default config directory for generic usage (`config.d`) and a thumbnail specific config directory (`thumbnail.d`).
You can edit the file contained in these directory or add your own in specific directories (see below).

The following command-line options <b> cannot </b> be set via config file:
`help`, `version`, `readers-list`, `config`, `dry-run` and `input`.

The following command-line options <b>are only taken on the first load</b>:
`no-render`, `output`, and all testing options.

Boolean options that have been turned on in the configuration file can be turned
off on the command line if needed, eg: `--point-sprites=false`.

F3D looks for configuration files in different locations depending on your operating system.
Existing configuration files are read in order and combined with later entries, potentially overriding previously read entry with the same names.

 * Linux: `/etc/f3d/config(.json,.d)`, `/usr/share/f3d/configs/config(.json,.d)`, `[install_dir]/share/f3d/configs/config(.json,.d)`, `${XDG_CONFIG_HOME}/f3d/config(.json,.d)`
 * Windows: `[install_dir]\share\f3d\configs\(config.json,.d)`, `%APPDATA%\f3d\(config.json,.d)`
 * macOS: `/usr/local/etc/f3d/config(.json,.d)`, `f3d.app/Contents/Resources/configs/config(.json,.d)`, `${XDG_CONFIG_HOME}/f3d/config(.json,.d)`

Please note `XDG_CONFIG_HOME` rely on environment variables as specified [here](https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html).

The binary release will install the default config directory.
On Linux, they will be installed in `[install_dir]/share/f3d/configs/`, on Windows, they will be installed in `[install_dir]\share\f3d\configs\`, on macOS, it will be installed in the bundle.

Please note there is a command line option to control the configuration file to read. Using it, one can specify an absolute/relative path for the configuration path, but also
only the filename or filestem (`.json` and `.d`  will be added) to look for in the locations listed above, , eg: `f3d --config=custom_config` will look
for `custom_config.json` and `custom_config.d` in locations listed above.
When specifying an absolute/relative path for the configuration file, a single file is read. If not, all files from locations listed above, with the overriding logic specified above.
