# Only internal build is supported for static plugins
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(FATAL_ERROR "native plugin cannot be built externally")
endif()

include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME 3DS
  EXTENSIONS 3ds
  MIMETYPES application/vnd.3ds
  VTK_IMPORTER vtk3DSImporter
  FORMAT_DESCRIPTION "Autodesk 3D Studio"
)

f3d_plugin_declare_reader(
  NAME CityGML
  EXTENSIONS gml
  MIMETYPES application/gml+xml
  VTK_READER vtkCityGMLReader
  FORMAT_DESCRIPTION "CityGML"
)

f3d_plugin_declare_reader(
  NAME DICOM
  EXTENSIONS dcm
  MIMETYPES application/dicom
  VTK_READER vtkDICOMImageReader
  FORMAT_DESCRIPTION "DICOM"
)

f3d_plugin_declare_reader(
  NAME GLTF
  SCORE 80
  EXTENSIONS gltf glb
  MIMETYPES model/gltf-binary model/gltf+json
  VTK_READER vtkGLTFReader
  VTK_IMPORTER vtkGLTFImporter
  FORMAT_DESCRIPTION "GL Transmission Format"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/gltf.inl"
)

f3d_plugin_declare_reader(
  NAME MetaImage
  EXTENSIONS mha mhd
  MIMETYPES application/vnd.mhd
  VTK_READER vtkMetaImageReader
  FORMAT_DESCRIPTION "MetaImage"
)

f3d_plugin_declare_reader(
  NAME Nrrd
  EXTENSIONS nrrd nhdr
  MIMETYPES application/vnd.nrrd
  VTK_READER vtkNrrdReader
  FORMAT_DESCRIPTION "Nearly Raw Raster Data"
)

f3d_plugin_declare_reader(
  NAME OBJ
  EXTENSIONS obj
  MIMETYPES model/obj
  VTK_READER vtkOBJReader
  VTK_IMPORTER vtkOBJImporter
  FORMAT_DESCRIPTION "Wavefront OBJ"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/obj.inl"
)

f3d_plugin_declare_reader(
  NAME PLYReader
  EXTENSIONS ply
  MIMETYPES application/vnd.ply
  VTK_READER vtkPLYReader
  FORMAT_DESCRIPTION "Polygon"
)

f3d_plugin_declare_reader(
  NAME PTS
  EXTENSIONS pts
  MIMETYPES application/vnd.pts
  VTK_READER vtkPTSReader
  FORMAT_DESCRIPTION "Point Cloud"
)

f3d_plugin_declare_reader(
  NAME STL
  EXTENSIONS stl
  MIMETYPES model/stl
  VTK_READER vtkSTLReader
  FORMAT_DESCRIPTION "Standard Triangle Language"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/stl.inl"
)

f3d_plugin_declare_reader(
  NAME TIFF
  EXTENSIONS tiff tif
  MIMETYPES application/x-tgif
  VTK_READER vtkTIFFReader
  FORMAT_DESCRIPTION "TIFF"
)

f3d_plugin_declare_reader(
  NAME VRMLReader
  EXTENSIONS wrl vrml
  MIMETYPES model/vrml
  VTK_IMPORTER vtkVRMLImporter
  FORMAT_DESCRIPTION "VRML"
)

f3d_plugin_declare_reader(
  NAME VTKLegacy
  SCORE 99
  EXTENSIONS vtk
  MIMETYPES application/vnd.vtk
  VTK_READER vtkPDataSetReader
  FORMAT_DESCRIPTION "VTK Legacy"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTU
  SCORE 99
  EXTENSIONS vtu
  MIMETYPES application/vnd.vtu
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML UnstructuredGrid"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTP
  SCORE 99
  EXTENSIONS vtp
  MIMETYPES application/vnd.vtp
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML PolyData"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTI
  SCORE 99
  EXTENSIONS vti
  MIMETYPES application/vnd.vti
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML ImageData"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTR
  SCORE 99
  EXTENSIONS vtr
  MIMETYPES application/vnd.vtr
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML RectangularGrid"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTS
  SCORE 99
  EXTENSIONS vts
  MIMETYPES application/vnd.vts
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML StructuredGrid"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTM
  SCORE 99
  EXTENSIONS vtm
  MIMETYPES application/vnd.vtm
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML MultiBlock"
)

f3d_plugin_build(
  NAME native
  VERSION 1.0
  DESCRIPTION "Native VTK I/O support"
  VTK_MODULES CommonCore CommonExecutionModel IOImport IOXML IOParallel IOCityGML IOImage IOGeometry IOPLY
  MIMETYPE_XML_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/f3d-3d-formats.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/f3d-3d-image-formats.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/f3d-vtk-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
  FORCE_STATIC
)
