# Supported File Formats

F3D supports the following file formats:

| Name                                      | File Extension(s)                              | Full scene | Animation Supported? | Plugin    | Reader Name             |
| ----------------------------------------- | ---------------------------------------------- | ---------- | -------------------- | --------- | ----------------------- |
| Legacy VTK                                | `.vtk`                                         | No         | No                   | `native`  | `VTKLegacy`             |
| VTK XML                                   | `.vtp`, `.vtu`, `.vtr`, `.vti`, `.vts`, `.vtm` | No         | No                   | `native`  | `VTKXMLVT<P,U,R,I,S,M>` |
| VTKHDF                                    | `.vtkhdf`                                      | No         | Yes                  | `hdf`     | `VTKHDF`                |
| EXODUS II                                 | `.e`, `.ex2`, `.exo`, `.g`                     | No         | Yes                  | `hdf`     | `ExodusII`              |
| Polygon File Format                       | `.ply`                                         | No         | No                   | `native`  | `PLYReader`             |
| Standard Triangle Language                | `.stl`                                         | No         | No                   | `native`  | `STL`                   |
| DICOM                                     | `.dcm`                                         | No         | No                   | `native`  | `DICOM`                 |
| NRRD ("nearly raw raster data")           | `.nrrd`, `.nhdr`                               | No         | No                   | `native`  | `Nrrd`                  |
| MetaHeader MetaIO                         | `.mhd`, `.mha`                                 | No         | No                   | `native`  | `MetaImage`             |
| Tag Image File Format 2D/3D               | `.tif`, `.tiff`                                | No         | No                   | `native`  | `TIFF`                  |
| QuakeMDL                                  | `.mdl`                                         | Yes        | Yes                  | `native`  | `QuakeMDL`              |
| CityGML                                   | `.gml`                                         | No         | No                   | `native`  | `CityGML`               |
| Point Cloud                               | `.pts`                                         | No         | No                   | `native`  | `PTS`                   |
| Standard for the Exchange of Product Data | `.step`, `.stp`, `.stpnc`, `.p21`, `.210`      | No         | No                   | `occt`    | `STEP`                  |
| Initial Graphics Exchange Specification   | `.iges`, `.igs`                                | No         | No                   | `occt`    | `IGES`                  |
| Open CASCADE Technology BRep format       | `.brep`                                        | No         | No                   | `native`  | `BREP`                  |
| Open CASCADE Technology XBF format        | `.xbf`                                         | No         | No                   | `occt`    | `XBF`                   |
| Alembic                                   | `.abc`                                         | No         | Yes                  | `alembic` | `Alembic`               |
| Wavefront OBJ                             | `.obj`                                         | Yes        | Yes                  | `native`  | `OBJ`                   |
| GL Transmission Format                    | `.gltf`, `.glb`                                | Yes        | Yes                  | `native`  | `GLTF`, `GLTFDraco`     |
| Draco                                     | `.drc`                                         | No         | No                   | `draco`   | `Draco`                 |
| Autodesk 3D Studio                        | `.3ds`                                         | Yes        | No                   | `native`  | `3DS`                   |
| Virtual Reality Modeling Language         | `.wrl`, `.vrml`                                | Yes        | No                   | `native`  | `VRMLReader`            |
| Autodesk Filmbox                          | `.fbx`                                         | Yes        | Yes                  | `assimp`  | `FBX`                   |
| COLLADA                                   | `.dae`                                         | Yes        | Yes                  | `assimp`  | `COLLADA`               |
| Object File Format                        | `.off`                                         | Yes        | No                   | `assimp`  | `OFF`                   |
| Drawing Exchange Format                   | `.dxf`                                         | Yes        | No                   | `assimp`  | `DXF`                   |
| DirectX                                   | `.x`                                           | Yes        | Yes                  | `assimp`  | `DirectX`               |
| 3D Manufacturing Format                   | `.3mf`                                         | Yes        | No                   | `assimp`  | `3MF`                   |
| Universal Scene Description               | `.usd`, `.usda`, `.usdc`, `.usdz`              | Yes        | Yes                  | `usd`     | `USD`                   |
| VDB                                       | `.vdb`                                         | No         | No                   | `vdb`     | `VDB`                   |
| 3D Gaussian splatting                     | `.splat`                                       | No         | No                   | `native`  | `Splat`                 |
| Compressed 3D Gaussian splatting          | `.spz`                                         | No         | No                   | `native`  | `SPZ`                   |

## Reader options

Readers can provide option that can be set using the `-D/--define` [command line option](OPTIONS.md).
eg: `-DVDB.downsampling_factor=0.5`.

For booleans, 0 means false, not 0 means true. Unsigned int will interpret anything that is not a non-negative integer as the default value.

| File extension | Option Name                | Argument Type  | Description                                                                          |
| -------------- | -------------------------- | -------------- | ------------------------------------------------------------------------------------ |
| `vdb`          | `VDB.downsampling_factor`  | `double`       | Control the level of downsampling when reading a volume, default is 0.1.             |
| `occt`         | `STEP.linear_deflection`   | `double`       | Control the distance between a curve and the resulting tessellation, default is 0.1. |
| `occt`         | `STEP.angular_deflection`  | `double`       | Control the angle between two subsequent segments, default is 0.5.                   |
| `occt`         | `STEP.relative_deflection` | `bool`         | Control if the deflection values are relative to object size, default is false.      |
| `occt`         | `STEP.read_wire`           | `bool`         | Control if lines should be read, default is true.                                    |
| `occt`         | `IGES.linear_deflection`   | `double`       | Control the distance between a curve and the resulting tessellation, default is 0.1. |
| `occt`         | `IGES.angular_deflection`  | `double`       | Control the angle between two subsequent segments, default is 0.5.                   |
| `occt`         | `IGES.relative_deflection` | `bool`         | Control if the deflection values are relative to object size, default is false.      |
| `occt`         | `IGES.read_wire`           | `bool`         | Control if lines should be read, default is true.                                    |
| `occt`         | `BREP.linear_deflection`   | `double`       | Control the distance between a curve and the resulting tessellation, default is 0.1. |
| `occt`         | `BREP.angular_deflection`  | `double`       | Control the angle between two subsequent segments, default is 0.5.                   |
| `occt`         | `BREP.relative_deflection` | `bool`         | Control if the deflection values are relative to object size, default is false.      |
| `occt`         | `BREP.read_wire`           | `bool`         | Control if lines should be read, default is true.                                    |
| `occt`         | `XBF.linear_deflection`    | `double`       | Control the distance between a curve and the resulting tessellation, default is 0.1. |
| `occt`         | `XBF.angular_deflection`   | `double`       | Control the angle between two subsequent segments, default is 0.5.                   |
| `occt`         | `XBF.relative_deflection`  | `bool`         | Control if the deflection values are relative to object size, default is false.      |
| `occt`         | `XBF.read_wire`            | `bool`         | Control if lines should be read, default is true.                                    |
| `mdl`          | `QuakeMDL.skin_index`      | `unsigned int` | Select a particular skin from a `mdl` file. Uses 0-indexing, default is 0.           |

## Format details

### QuakeMDL

- Models texture are loaded with a simple PBR lighting (diffuse color only, no specular, index of refraction set to 1.0).
- Selecting `skin` is not supported.
- Loading `groupskin` is not supported.
- Animation frames are split based on their names, eg: `stand1`, `stand2`, `stand3`, `run1`, `run2`, `run3`.
