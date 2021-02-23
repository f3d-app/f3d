title: Configuration file
---

All the command-line options can be controlled using a configuration file.
This configuration file uses the "long" version of the options in a JSON
formatted file to provide default values for these options.

These options can be organized by block using a regular expression for each block
in order to provide different default values for the different filetypes.

Using a command-line option will override the corresponding value in the config file.
A typical config file may look like this:

```javascript
{
   ".*": {
       "resolution": "1200,800",
       "bg-color": "0.7,0.7,0.7",
       "color": "0.5,0.1,0.1",
       "fxaa": true,
       "timer": true,
       "progress": true,
       "axis": true,
       "bar": true,
       "verbose": true,
       "roughness": 0.2,
       "grid": true
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

The configuration file possible locations depends on your operating system.
They are considered in the below order and only the first found will be used.

 * Linux: `/etc/f3d/config.json`, `[install_dir]/config.json`, `${XDG_CONFIG_HOME}/.config/f3d/config.json`, `~/.config/f3d/config.json`
 * Windows: `[install_dir]\config.json`, `%APPDATA%\f3d\config.json`
 * macOS: `/etc/f3d/config.json`, `f3d.app/Contents/Resources/config.json`, `[install_dir]/config.json`, `~/.config/f3d/config.json`

If you are using the releases, a default configuration file is provided when installing F3D.
On Linux, it will be installed in `/etc/f3d/`, on Windows, it will be installed in the install directory, on macOS, it will be installed in the bundle.
