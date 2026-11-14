# Final shader examples

Load a shader file with `--final-shader`:

| File | Effect |
| --- | --- |
| `outline.glsl` | Binary depth contours over the model colors, with area opening and adjustable thickness. |
| `comics.glsl` | The same contours with depth-driven halftone dots. |
| `depth-preview.glsl` | Linear-depth preview, blue at 0 and red at 100 model units. |
| `depth-test.glsl` | Reference depth sample, blue at 500 and red at 2000 model units. |
| `depth-fog.glsl` | Fog based on the visible depth range. |
| `depth-gradient-bw.glsl` | Grayscale depth gradient. |
| `depth-gradient-color.glsl` | Color depth gradient. |
| `scanner.glsl` | Scanning effect. |

## Color and depth outlines

Run from the repository root with the local F3D build:

```bash
GALLIUM_DRIVER=d3d12 ./build/bin/f3d \
  --rendering-backend=glx \
  --final-shader "$(cat examples/shaders/outline.glsl)" \
  /path/to/model.glb
```

`GALLIUM_DRIVER=d3d12` selects GPU rendering under WSLg. On other platforms,
omit that variable and select a supported rendering backend.

Use the build from `hackathon/shaders` with the depth sampling helpers integrated
from `hackathon/depth-map`. It provides `readDepth(uv)` for linear camera-space
depth and `readRawDepth(uv)` for normalized depth, alongside `source`, `sourceDepth`,
`cameraClippingRange`, `cameraParallelProjection` and `resolution`.
Rebuild once after switching branches:

```bash
cmake --build build --parallel 4
```

Do not use `--display-depth` with `outline.glsl` or `comics.glsl`: keep the normal rendering path
so both scene color and scene depth are available. Tone mapping and anti-aliasing
can stay enabled because edge detection samples the separate depth texture.

The shader follows `depth-test.glsl`: use `readRawDepth(uv)` to identify the
background, then `readDepth(uv)` for linear camera-space depth. Neither shader
reimplements depth sampling or linearization.
Neighbor coordinates are clamped to the depth texture bounds to avoid wrapping
at screen edges when the model fills the view.

The outline shader compares
opposite neighbors at a fixed one-pixel spacing to detect changes in depth slope,
using second differences rather than a first-derivative Sobel filter.

For a projected plane, inverse camera-space depth (`1/z`) varies affinely in
perspective, while camera-space depth (`z`) varies affinely in orthographic.
Using the corresponding representation cancels a regular planar slope, including
at grazing angles. The second differences are normalized by the center value.
The threshold therefore measures a relative departure from a planar surface,
rather than a large depth gradient or an absolute distance in model units.

A binary `step` selects the detected edges. An optional area opening removes
8-connected components containing fewer than `OPENING_MIN_PIXELS` pixels before
applying the line thickness. Unlike a square erosion/dilation opening, this keeps
thin curves and their endpoints when the connected component is large enough.
The shader explores at most the selected number of foreground pixels per query.
Larger minimum sizes require more work; keep this value small. Short legitimate
details below the selected area are removed too.

A small square dilation kernel then widens the cleaned binary mask. Width 1
leaves that mask unchanged; higher widths only expand the existing lines.
They do not change the depth sampling or detection threshold.
There is no thinning stage. Lines can expand onto the background; thick lines can
merge nearby details. This does not guarantee an exact geometric thickness for
diagonals or edges already covering several pixels.

Restart F3D after editing the shader to load the updated file. Shader edits do
not require recompilation. F3D supplies the GLSL version and uniforms; the file
only defines helper functions, constants and `pixel`.

Both shaders share the contour implementation and these constants:

| Constant | Effect |
| --- | --- |
| `OUTLINE_WIDTH` | Size of the binary dilation kernel in pixels, rounded to an integer. Use 1 for the original mask, 2 or more to widen it, or 0 to disable outlines. |
| `OPENING_ENABLED` | Enable the area opening before thickening contours. |
| `OPENING_MIN_PIXELS` | Minimum component size in raw mask pixels, before line thickening. Default 8 removes components of 1–7 pixels. Use an integer of at least 1. |
| `DEPTH_THRESHOLD` | Minimum normalized change in depth slope that starts a contour. Lower values show more internal details. |
| `INK_COLOR` | Line color. |
| `OUTLINE_ONLY` | Set to `true` for an opaque binary ink-on-white diagnostic view, or `false` to keep the model colors. |

## Depth-driven halftone

Use `comics.glsl` to add dots to the same contours:

```bash
GALLIUM_DRIVER=d3d12 ./build/bin/f3d \
  --rendering-backend=glx \
  --final-shader "$(cat examples/shaders/comics.glsl)" \
  /path/to/model.glb
```

`outline.glsl` contains no halftone code. `comics.glsl` draws fixed-size dots on a
fixed screen-space grid. Their
number increases as the model gets closer, using normalized inverse camera-space
depth (`1/z`). A deterministic rank selects
each grid cell according to its center's depth, so a selected cell contains a
complete dot. Foreground depth clips dots at the model boundary. RGB values are
used only for compositing, never for deciding where dots appear.

| Constant | Effect |
| --- | --- |
| `HALFTONE_ENABLED` | Enable the depth-driven dots. Set to `false` to restore contours alone. |
| `HALFTONE_CELL_SIZE` | Grid spacing in screen pixels. Default 6. |
| `HALFTONE_RADIUS` | Fixed dot radius in pixels. Default 1.35. Does not vary with depth. |
| `HALFTONE_DEPTH_START` | Positive depth in scene units where every grid cell is selected. Default 15. Closer surfaces keep this maximum density. |
| `HALFTONE_DEPTH_END` | Depth where density reaches zero. Default 35. Must exceed the start distance. |

The 15–35 range is a starting point for the bus model; adjust it to your model's
distances and scale. Dot edges are softened over one pixel. The cell selection
does not depend on time, but cells can switch on or off as the camera changes
their depth. The grid stays fixed on screen rather than following the surface.

The area opening affects only the contour mask. Dots are composited separately
before the contours, and are hidden in `OUTLINE_ONLY` diagnostic mode.

Window openings and internal occlusions can produce contours. This experiment
does not classify meshes or detect every crease between touching surfaces.
The depth texture contains the scene depth published by the main render pass;
transparency does not guarantee a depth sample for every visible layer.

## Debug shaders

- `depth-preview.glsl`: the supplied linear-depth visualization, blue at 0 and
  red at 100 model units, with a black background. Run it with the normal rendering
  path and the new uniforms, without `--display-depth`.
