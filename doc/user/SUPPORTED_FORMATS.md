# Supported File Formats

F3D supports the following file formats:

| Name                                      | File Extension(s)                              | Full scene | Animation Supported? | Plugin    |
| ----------------------------------------- | ---------------------------------------------- | ---------- | -------------------- | --------- |
| Legacy VTK                                | `.vtk`                                         | No         | No                   | `native`  |
| VTK XML                                   | `.vtp`, `.vtu`, `.vtr`, `.vti`, `.vts`, `.vtm` | No         | No                   | `native`  |
| Polygon File Format                       | `.ply`                                         | No         | No                   | `native`  |
| Standard Triangle Language                | `.stl`                                         | No         | No                   | `native`  |
| DICOM                                     | `.dcm`                                         | No         | No                   | `native`  |
| NRRD ("nearly raw raster data")           | `.nrrd`, `.nhrd`                               | No         | No                   | `native`  |
| MetaHeader MetaIO                         | `.mhd`, `.mha`                                 | No         | No                   | `native`  |
| Tag Image File Format 2D/3D               | `.tif`, `.tiff`                                | No         | No                   | `native`  |
| EXODUS II                                 | `.e`, `.ex2`, `.exo`, `.g`                     | No         | Yes                  | `exodus`  |
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

\***Note:** As of version 2.5.0, F3D support for VDB is still experimental.
