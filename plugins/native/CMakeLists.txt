include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME 3DS
  EXTENSIONS 3ds
  MIMETYPES application/vnd.3ds
  VTK_IMPORTER vtk3DSImporter
  DESCRIPTION "3DS files reader"
)

f3d_plugin_declare_reader(
  NAME CityGML
  EXTENSIONS gml
  MIMETYPES application/gml+xml
  VTK_READER vtkCityGMLReader
  DESCRIPTION "CityGML files reader"
)

f3d_plugin_declare_reader(
  NAME DICOM
  EXTENSIONS dcm
  MIMETYPES application/dicom
  VTK_READER vtkDICOMImageReader
  DESCRIPTION "DICOM images files reader"
)

f3d_plugin_declare_reader(
  NAME GLTF
  SCORE 80
  EXTENSIONS gltf glb
  MIMETYPES model/gltf-binary model/gltf+json
  VTK_READER vtkGLTFReader
  VTK_IMPORTER vtkGLTFImporter
  DESCRIPTION "glTF files reader"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/gltf.inl"
)

f3d_plugin_declare_reader(
  NAME MetaImage
  EXTENSIONS mha mhd
  MIMETYPES application/vnd.mhd
  VTK_READER vtkMetaImageReader
  DESCRIPTION "MetaImage files reader"
)

f3d_plugin_declare_reader(
  NAME Nrrd
  EXTENSIONS nrrd nhdr
  MIMETYPES application/vnd.nrrd
  VTK_READER vtkNrrdReader
  DESCRIPTION "Nrrd files reader"
)

f3d_plugin_declare_reader(
  NAME OBJ
  EXTENSIONS obj
  MIMETYPES model/obj
  VTK_READER vtkOBJReader
  VTK_IMPORTER vtkOBJImporter
  DESCRIPTION "OBJ files reader"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/obj.inl"
)

f3d_plugin_declare_reader(
  NAME PLYReader
  EXTENSIONS ply
  MIMETYPES application/vnd.ply
  VTK_READER vtkPLYReader
  DESCRIPTION "PLY files reader"
)

f3d_plugin_declare_reader(
  NAME PTS
  EXTENSIONS pts
  MIMETYPES application/vnd.pts
  VTK_READER vtkPTSReader
  DESCRIPTION "PTS files reader"
)

f3d_plugin_declare_reader(
  NAME STL
  EXTENSIONS stl
  MIMETYPES model/stl
  VTK_READER vtkSTLReader
  DESCRIPTION "STL files reader"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/stl.inl"
)

f3d_plugin_declare_reader(
  NAME TIFF
  EXTENSIONS tiff tif
  MIMETYPES application/x-tgif
  VTK_READER vtkTIFFReader
  DESCRIPTION "TIFF files reader"
)

f3d_plugin_declare_reader(
  NAME VRMLReader
  EXTENSIONS wrl vmrl
  MIMETYPES model/vrml
  VTK_IMPORTER vtkVRMLImporter
  DESCRIPTION "VRML files reader"
)

f3d_plugin_declare_reader(
  NAME VTKLegacy
  SCORE 99
  EXTENSIONS vtk
  MIMETYPES application/vnd.vtk
  VTK_READER vtkPDataSetReader
  DESCRIPTION "VTK Legacy files reader"
)

f3d_plugin_declare_reader(
  NAME VTKXML
  SCORE 99
  EXTENSIONS vtp vtu vtr vti vts vtm
  MIMETYPES application/vnd.vtp application/vnd.vtu application/vnd.vtr application/vnd.vti application/vnd.vts application/vnd.vtm
  VTK_READER vtkXMLGenericDataObjectReader
  DESCRIPTION "VTK XML files reader"
)

f3d_plugin_build(
  NAME native
  VERSION 1.0
  DESCRIPTION "Native VTK I/O support"
  VTK_MODULES CommonCore CommonExecutionModel IOImport IOXML IOParallel IOCityGML IOImage IOGeometry IOPLY
  FORCE_STATIC
)
