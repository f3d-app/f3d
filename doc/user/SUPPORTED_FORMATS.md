# Supported File Formats

F3D supports the following file formats:

| Name | File Extension(s) | Type of Scene(s) Supported | Animations Supported? | Plugin Supported |
| -- | -- | -- | -- | -- |
| Legacy VTK | `.vtk` | Default | 
| VTK XML | `.vtp`, `.vtu`, `.vtr`, `.vti`, `.vts`, `.vtm` | Default |
| Polygon File Format | `.ply` | Default |
| Standard Triangle Language | `.stl` | Default |  
| DICOM | `.dcm` | Default |
| NRRD ("nearly raw raster data") | `.nrrd`, `.nhrd` | Default |
| MetaHeader MetaIO | `.mhd`, `.mha` | Default |
| Tag Image File Format 2D/3D | `.tif`, `.tiff` | Default |
| EXODUS II | `.e`, `.ex2`, `.exo`, `.g` | Default | Yes | `exodus` |
| CityGML | `.gml` | Default |
| Point Cloud | `.pts` | Default |
| Standard for the Exchange of Product Data | `.step`, `.stp` | Default | | `occt` |
| Initial Graphics Exchange Specification | `.iges`, `.igs` | Default | | `occt` |
| Open CASCADE Technology BRep format | `.brep` | Default |
| Alembic | `.abc` | Default | |  `alembic` |
| Wavefront OBJ | `.obj` | Default, Full |
| GL Transmission Format | `.gltf`, `.glb` | Default, Full | Yes |
| Autodesk 3D Studio | `.3ds` | Full |
| Virtual Reality Modeling Language | `.wrl` | Full |
| Autodesk Filmbox | `.fbx` | Full | Yes | `assimp` |
| COLLADA | `.dae` | Full | Yes | `assimp` |
| Object File Format | `.off` | Full | | `assimp` |
| Drawing Exchange Format | `.dxf` | Full | | `assimp` |
| DirectX | `.x` | Full | Yes | `assimp` |
| 3D Manufacturing Format | `.3mf` | Full | | `assimp` |
| Universal Scene Description | `.usd`, `.usda`, `.usdc`, `.usdz` | Full | Yes | `usd` |
| VDB | `.vdb` | Default | | `vdb` |

\***Note:** As of version 2.5.0, F3D support for VDB is still experimental.
