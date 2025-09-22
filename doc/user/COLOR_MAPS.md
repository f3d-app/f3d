# Color maps

## Image files

It is possible to specify a color map using the `--colormap-file` option.
The value of the option can be an absolute path to an image file, a relative path, a filename or a filestem.
If it is not an absolute path, these directories are used to find the file (in this order):

- Linux: `${XDG_CONFIG_HOME}/f3d/colormaps`, `~/.config/f3d/colormaps`, `/etc/f3d/colormaps`, `/usr/share/f3d/colormaps`, `[install_dir]/share/f3d/colormaps`
- Windows: `%APPDATA%\f3d\colormaps`, `[install_dir]\share\f3d\configs\colormaps`
- macOS: `${XDG_CONFIG_HOME}/f3d/colormaps`, `~/.config/f3d/colormaps`, `/usr/local/etc/f3d/colormaps`, `f3d.app/Contents/Resources/configs/colormaps`
  The first existing file found is used.
  If it is a filestem, all [supported image](#supported-formats) extensions are tried.

We provide some ready to use color maps files, listed in the table below:

| Name       | Image                                                                                                                                   |
| ---------- | --------------------------------------------------------------------------------------------------------------------------------------- |
| cividis    | <img src="https://github.com/f3d-app/f3d/blob/master/resources/colormaps/cividis.png?raw=true" class="cm" width="256" height="10" />    |
| cubehelix  | <img src="https://github.com/f3d-app/f3d/blob/master/resources/colormaps/cubehelix.png?raw=true" class="cm" width="256" height="10" />  |
| gist_earth | <img src="https://github.com/f3d-app/f3d/blob/master/resources/colormaps/gist_earth.png?raw=true" class="cm" width="256" height="10" /> |
| hot        | <img src="https://github.com/f3d-app/f3d/blob/master/resources/colormaps/hot.png?raw=true" class="cm" width="256" height="10" />        |
| inferno    | <img src="https://github.com/f3d-app/f3d/blob/master/resources/colormaps/inferno.png?raw=true" class="cm" width="256" height="10" />    |
| magma      | <img src="https://github.com/f3d-app/f3d/blob/master/resources/colormaps/magma.png?raw=true" class="cm" width="256" height="10" />      |
| plasma     | <img src="https://github.com/f3d-app/f3d/blob/master/resources/colormaps/plasma.png?raw=true" class="cm" width="256" height="10" />     |
| seismic    | <img src="https://github.com/f3d-app/f3d/blob/master/resources/colormaps/seismic.png?raw=true" class="cm" width="256" height="10" />    |
| viridis    | <img src="https://github.com/f3d-app/f3d/blob/master/resources/colormaps/viridis.png?raw=true" class="cm" width="256" height="10" />    |

It is possible to create a custom color map by creating a simple RGB image in any [supported formats](#supported-formats), and any resolution (if the height is more than 1 row, only the first one is taken into account). The image must be copied in the user config directory:

- Linux/macOS: `~/.config/f3d/colormaps`
- Windows: `%APPDATA%\f3d\colormaps`

### Supported formats

Here's the list of all supported image formats that can be used as color maps:

- `.png`
- `.pnm`, `.pgm`, `.ppm`
- `.tif`, `.tiff`
- `.bmp`
- `.slc`
- `.hdr`
- `.pic`
- `.jpeg`, `.jpg`
- `.MR`
- `.CT`
- `.mhd`, `.mha`
- `.tga`
- `.exr` (if `F3D_MODULE_EXR` is [enabled](../dev/BUILD.md))
- `.webp` (if `F3D_MODULE_WEBP` is [enabled](../dev/BUILD.md))

## Custom values

If no colormap file is specified, it is also possible to set values manually using the `--colormap` option with a dedicated [parsing](PARSING.md#colormap).
For example, the default value corresponds to the `hot` preset which can be defined manually with `--colormap=0.0,rgb(0,0,0),0.4,rgb(230,0,0),0.8,rgb(230,230,0),1.0,rgb(255,255,255)`.
It consists of 4 pairs of increasing scalar values ranging from 0 to 1, and corresponding colors. Values in between are interpolated.
