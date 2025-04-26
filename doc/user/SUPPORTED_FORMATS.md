# Supported File Formats

F3D supports the following file formats:

| Name                                      | File Extension(s)                              | Full scene | Animation Supported? | Plugin    |
| ----------------------------------------- | ---------------------------------------------- | ---------- | -------------------- | --------- |
| Legacy VTK                                | `.vtk`                                         | No         | No                   | `native`  |
| VTK XML                                   | `.vtp`, `.vtu`, `.vtr`, `.vti`, `.vts`, `.vtm` | No         | No                   | `native`  |
| VTKHDF                                    | `.vtkhdf`                                      | No         | Yes                  | `hdf`     |
| EXODUS II                                 | `.e`, `.ex2`, `.exo`, `.g`                     | No         | Yes                  | `hdf`     |
| Polygon File Format                       | `.ply`                                         | No         | No                   | `native`  |
| Standard Triangle Language                | `.stl`                                         | No         | No                   | `native`  |
| DICOM                                     | `.dcm`                                         | No         | No                   | `native`  |
| NRRD ("nearly raw raster data")           | `.nrrd`, `.nhrd`                               | No         | No                   | `native`  |
| MetaHeader MetaIO                         | `.mhd`, `.mha`                                 | No         | No                   | `native`  |
| Tag Image File Format 2D/3D               | `.tif`, `.tiff`                                | No         | No                   | `native`  |
| QuakeMDL                                  | `.mdl`                                         | Yes        | Yes                  | `native`  |
| CityGML                                   | `.gml`                                         | No         | No                   | `native`  |
| Point Cloud                               | `.pts`                                         | No         | No                   | `native`  |
| Standard for the Exchange of Product Data | `.step`, `.stp`                                | No         | No                   | `occt`    |
| Initial Graphics Exchange Specification   | `.iges`, `.igs`                                | No         | No                   | `occt`    |
| Open CASCADE Technology BRep format       | `.brep`                                        | No         | No                   | `native`  |
| Alembic                                   | `.abc`                                         | No         | Yes                  | `alembic` |
| Wavefront OBJ                             | `.obj`                                         | Yes        | Yes                  | `native`  |
| GL Transmission Format                    | `.gltf`, `.glb`                                | Yes        | Yes                  | `native`  |
| Autodesk 3D Studio                        | `.3ds`                                         | Yes        | No                   | `native`  |
| Virtual Reality Modeling Language         | `.wrl`                                         | Yes        | No                   | `native`  |
| Autodesk Filmbox                          | `.fbx`                                         | Yes        | Yes                  | `assimp`  |
| COLLADA                                   | `.dae`                                         | Yes        | Yes                  | `assimp`  |
| Object File Format                        | `.off`                                         | Yes        | No                   | `assimp`  |
| Drawing Exchange Format                   | `.dxf`                                         | Yes        | No                   | `assimp`  |
| DirectX                                   | `.x`                                           | Yes        | Yes                  | `assimp`  |
| 3D Manufacturing Format                   | `.3mf`                                         | Yes        | No                   | `assimp`  |
| Universal Scene Description               | `.usd`, `.usda`, `.usdc`, `.usdz`              | Yes        | Yes                  | `usd`     |
| VDB                                       | `.vdb`                                         | No         | No                   | `vdb`     |

## Reader options

Readers can provide option that can be set using the `-D/--define` [command line option](OPTIONS.md).
eg: `-DVDB.downsampling_factor=0.5`.

Values are parsed as doubles. For booleans, 0 means false, not 0 means true.

- `vdb` - `VDB.downsampling_factor` : Control the level of downsampling when reading a volume, default is 0.1.
- `occt` - `STEP.linear_deflection` : Control the distance between a curve and the resulting tessellation, default is 0.1.
- `occt` - `STEP.angular_deflection` : Control the angle between two subsequent segments, default is 0.5.
- `occt` - `STEP.relative_deflection` : Control if the deflection values are relative to object size, default is false.
- `occt` - `STEP.read_wire` : Control if lines should be read, default is true.
- `occt` - `IGES.linear_deflection` : Control the distance between a curve and the resulting tessellation, default is 0.1.
- `occt` - `IGES.angular_deflection` : Control the angle between two subsequent segments, default is 0.5.
- `occt` - `IGES.relative_deflection` : Control if the deflection values are relative to object size, default is false.
- `occt` - `IGES.read_wire` : Control if lines should be read, default is true.
- `occt` - `BREP.linear_deflection` : Control the distance between a curve and the resulting tessellation, default is 0.1.
- `occt` - `BREP.angular_deflection` : Control the angle between two subsequent segments, default is 0.5.
- `occt` - `BREP.relative_deflection` : Control if the deflection values are relative to object size, default is false.
- `occt` - `BREP.read_wire` : Control if lines should be read, default is true.
- `occt` - `XBF.linear_deflection` : Control the distance between a curve and the resulting tessellation, default is 0.1.
- `occt` - `XBF.angular_deflection` : Control the angle between two subsequent segments, default is 0.5.
- `occt` - `XBF.relative_deflection` : Control if the deflection values are relative to object size, default is false.
- `occt` - `XBF.read_wire` : Control if lines should be read, default is true.

## Format details

### QuakeMDL

- Models texture are loaded with a simple PBR lighting (diffuse color only, no specular, index of refraction set to 1.0).
- Selecting `skin` is not supported.
- Loading `groupskin` is not supported.
- Animation frames are split based on their names, eg: `stand1`, `stand2`, `stand3`, `run1`, `run2`, `run3`.
