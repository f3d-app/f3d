title: Configuration file
---

All the command-line options can be controlled using a configuration file.
This configuration file uses the "long" version of the options in a JSON
formatted file to provide default values for these options.

These options can be organized by block using a regular expression for each block
in order to provide different default values for the different filetypes.

Using a command-line option will override the corresponding value in the config file.
A typical config file may look like this :

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
   ".*gl[tf|b]": {
       "raytracing": true,
       "denoise": true,
       "samples": 3
   }
}
```
This first block defines a basic configuration with many desired options for all files.
The second block specifies that all files ending with vt., eg: vtk, vtp, vtu, ... will be shown with edges on.
The third block specifies raytracing usage for .gltf and .glb files.

The configuration file location depends on your operating system:
 * Linux : `${XDG_CONFIG_HOME}/.config/f3d/f3d.json` if the variable exists, if not `~/.config/f3d/f3d.json`
 * Windows : `%APPDATA%\f3d\f3d.json`
 * MacOS : `~/.config/f3d/f3d.json`
 