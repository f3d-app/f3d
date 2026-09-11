# Color maps

## Image files

Specify a color map using the `--colormap-file` option.
Set the option value to an absolute path to an image file, a relative path, a filename or a filestem.
If it is not an absolute path, F3D uses these directories to find the file (in this order):

- Linux: `${XDG_CONFIG_HOME}/f3d/colormaps`, `~/.config/f3d/colormaps`, `/etc/f3d/colormaps`, `/usr/share/f3d/colormaps`, `[install_dir]/share/f3d/colormaps`
- Windows: `%APPDATA%\f3d\colormaps`, `[install_dir]\share\f3d\configs\colormaps`
- macOS: `${XDG_CONFIG_HOME}/f3d/colormaps`, `~/.config/f3d/colormaps`, `/usr/local/etc/f3d/colormaps`, `f3d.app/Contents/Resources/configs/colormaps`
  F3D uses the first existing file found.
  If it is a filestem, F3D tries all [supported image formats](#supported-formats).

F3D provides some ready to use color maps files, listed in the table below:

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

Create a custom color map with a simple RGB image in any [supported formats](#supported-formats), and any resolution. If the height is more than 1 row, F3D uses only the first row. Copy the image in the user config directory:

- Linux/macOS: `~/.config/f3d/colormaps`
- Windows: `%APPDATA%\f3d\colormaps`

### Supported formats

F3D supports these image formats as color maps:

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
- `.exr` (if `F3D_MODULE_EXR` is enabled)
- `.webp` (if `F3D_MODULE_WEBP` is enabled)

## Custom values

Alternatively, set values manually using the `--colormap` option with the dedicated [colormap parsing](08-PARSING.md#colormap) syntax.
For example, the default value corresponds to the `hot` preset. Define it manually with `--colormap=0.0,rgb(0,0,0),0.4,rgb(230,0,0),0.8,rgb(230,230,0),1.0,rgb(255,255,255)`.
It consists of 4 pairs of increasing scalar values ranging from 0 to 1, and corresponding colors. F3D interpolates values in between.
