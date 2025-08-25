# Supported File Formats

F3D supports the following file formats:

| Name                                      | File Extension(s)                              | Full scene | Animation Support | Plugin    | Reader Name             |
| ----------------------------------------- | ---------------------------------------------- | ---------- | ----------------- | --------- | ----------------------- |
| Legacy VTK                                | `.vtk`                                         | No         | NONE              | `native`  | `VTKLegacy`             |
| VTK XML                                   | `.vtp`, `.vtu`, `.vtr`, `.vti`, `.vts`, `.vtm` | No         | NONE              | `native`  | `VTKXMLVT<P,U,R,I,S,M>` |
| VTKHDF                                    | `.vtkhdf`                                      | No         | UNIQUE            | `hdf`     | `VTKHDF`                |
| EXODUS II                                 | `.e`, `.ex2`, `.exo`, `.g`                     | No         | UNIQUE            | `hdf`     | `ExodusII`              |
| NetCDF                                    | `.nc`, `.cdf`, `.ncdf`                         | No         | UNIQUE            | `hdf`     | `NetCDF`                |
| Polygon File Format                       | `.ply`                                         | No         | NONE              | `native`  | `PLYReader`             |
| Standard Triangle Language                | `.stl`                                         | No         | NONE              | `native`  | `STL`                   |
| DICOM                                     | `.dcm`                                         | No         | NONE              | `native`  | `DICOM`                 |
| NRRD ("nearly raw raster data")           | `.nrrd`, `.nhdr`                               | No         | NONE              | `native`  | `Nrrd`                  |
| MetaHeader MetaIO                         | `.mhd`, `.mha`                                 | No         | NONE              | `native`  | `MetaImage`             |
| Tag Image File Format 2D/3D               | `.tif`, `.tiff`                                | No         | NONE              | `native`  | `TIFF`                  |
| QuakeMDL                                  | `.mdl`                                         | Yes        | SINGLE            | `native`  | `QuakeMDL`              |
| CityGML                                   | `.gml`                                         | No         | NONE              | `native`  | `CityGML`               |
| Point Cloud                               | `.pts`                                         | No         | NONE              | `native`  | `PTS`                   |
| Standard for the Exchange of Product Data | `.step`, `.stp`, `.stpnc`, `.p21`, `.210`      | No         | NONE              | `occt`    | `STEP`                  |
| Initial Graphics Exchange Specification   | `.iges`, `.igs`                                | No         | NONE              | `occt`    | `IGES`                  |
| Open CASCADE Technology BRep format       | `.brep`                                        | No         | NONE              | `occt`    | `BREP`                  |
| Open CASCADE Technology XBF format        | `.xbf`                                         | No         | NONE              | `occt`    | `XBF`                   |
| Alembic                                   | `.abc`                                         | No         | UNIQUE            | `alembic` | `Alembic`               |
| Wavefront OBJ                             | `.obj`                                         | Yes        | NONE              | `native`  | `OBJ`                   |
| GL Transmission Format                    | `.gltf`, `.glb`                                | Yes        | MULTI             | `native`  | `GLTF`, `GLTFDraco`     |
| Draco                                     | `.drc`                                         | No         | NONE              | `draco`   | `Draco`                 |
| Autodesk 3D Studio                        | `.3ds`                                         | Yes        | NONE              | `native`  | `3DS`                   |
| Virtual Reality Modeling Language         | `.wrl`, `.vrml`                                | Yes        | NONE              | `native`  | `VRMLReader`            |
| Autodesk Filmbox                          | `.fbx`                                         | Yes        | SINGLE            | `assimp`  | `FBX`                   |
| COLLADA                                   | `.dae`                                         | Yes        | SINGLE            | `assimp`  | `COLLADA`               |
| Object File Format                        | `.off`                                         | Yes        | NONE              | `assimp`  | `OFF`                   |
| Drawing Exchange Format                   | `.dxf`                                         | Yes        | NONE              | `assimp`  | `DXF`                   |
| DirectX                                   | `.x`                                           | Yes        | SINGLE            | `assimp`  | `DirectX`               |
| 3D Manufacturing Format                   | `.3mf`                                         | Yes        | NONE              | `assimp`  | `3MF`                   |
| Universal Scene Description               | `.usd`, `.usda`, `.usdc`, `.usdz`              | Yes        | SINGLE            | `usd`     | `USD`                   |
| VDB                                       | `.vdb`                                         | No         | NONE              | `vdb`     | `VDB`                   |
| 3D Gaussian splatting                     | `.splat`                                       | No         | NONE              | `native`  | `Splat`                 |
| Compressed 3D Gaussian splatting          | `.spz`                                         | No         | NONE              | `native`  | `SPZ`                   |

## Reader options

Readers can provide option that can be set using the `-D/--define` [command line option](OPTIONS.md).
eg: `-DVDB.downsampling_factor=0.5` or using the `set_reader_option` [command](COMMANDS.md).

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
- Animation frames are split based on their names, eg: `stand1`, `stand2`, `stand3`, `run1`, `run2`, `run3`.

### 3D Gaussian splatting

Currently, 3 different formats are supported by F3D:

- `.ply`: Original 3DGS format
- `.splat`: Format specified by https://github.com/antimatter15/splat. Does not support spherical harmonics.
- `.spz`: Niantic's format specified by https://github.com/nianticlabs/spz

Note that no config files come with the `.ply` format because this format isn't dedicated to 3DGS only so we cannot generalize.
If you are using `.ply` for 3DGS only, you can set up a config file similar to what is done for `.splat` or `.spz`.
See configuration file [documentation](./CONFIGURATION_FILE.md)
