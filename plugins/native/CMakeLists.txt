cmake_minimum_required(VERSION 3.21)

project(f3d-plugin-native)

include(GNUInstallDirs)

# Only internal build is supported for static plugins
if(PROJECT_IS_TOP_LEVEL)
  message(FATAL_ERROR "native plugin cannot be built externally")
endif()

include(f3dPlugin)

f3d_plugin_init()

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20250925)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME 3DS
  EXTENSIONS 3ds
  MIMETYPES application/vnd.3ds
  VTK_IMPORTER vtk3DSImporter
  FORMAT_DESCRIPTION "Autodesk 3D Studio"
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/3ds.inl"
)

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251208)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME CityGML
  EXTENSIONS gml
  MIMETYPES application/gml+xml
  VTK_READER vtkCityGMLReader
  FORMAT_DESCRIPTION "CityGML"
  SCORE 30 # CanReadFile is just a XML check
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
)

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251210)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME DICOM
  EXTENSIONS dcm
  MIMETYPES application/dicom
  VTK_READER vtkDICOMImageReader
  FORMAT_DESCRIPTION "DICOM"
  ${_SUPPORTS_STREAM}
  CAN_READ MEMBER
)

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20250923)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME GLTF
  SCORE 60 # Higher priority than draco
  EXTENSIONS gltf
  MIMETYPES model/gltf+json
  VTK_IMPORTER vtkF3DGLTFImporter
  FORMAT_DESCRIPTION "GL Transmission Format"
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
)

# GLB reader will be removed once VTK v9.6 support is dropped
f3d_plugin_declare_reader(
  NAME GLB
  SCORE 60 # Higher priority than draco
  EXTENSIONS glb
  MIMETYPES model/gltf-binary
  VTK_IMPORTER vtkF3DGLTFImporter
  FORMAT_DESCRIPTION "GL Transmission Format (binary)"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/glb.inl"
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
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

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.20260103)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME OBJ
  EXTENSIONS obj
  MIMETYPES model/obj
  VTK_IMPORTER vtkOBJImporter
  FORMAT_DESCRIPTION "Wavefront OBJ"
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/obj.inl"
)

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251109)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME PTS
  EXTENSIONS pts
  MIMETYPES application/vnd.pts
  VTK_READER vtkPTSReader
  FORMAT_DESCRIPTION "Point Cloud"
  SCORE 30 # CanReadFile can be false positive with random ascii
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
)

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251104)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME STL
  EXTENSIONS stl
  MIMETYPES model/stl
  VTK_READER vtkSTLReader
  FORMAT_DESCRIPTION "Standard Triangle Language"
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/stl.inl"
)

f3d_plugin_declare_reader(
  NAME TIFF
  EXTENSIONS tiff tif
  MIMETYPES application/x-tgif
  VTK_READER vtkTIFFReader
  FORMAT_DESCRIPTION "TIFF"
  EXCLUDE_FROM_THUMBNAILER
)

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.20260106)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

set(_image_formats "png;jpeg;bmp;tga;hdr")
if(F3D_MODULE_WEBP)
  list(APPEND _image_formats "webp")
endif()
if(F3D_MODULE_EXR)
  list(APPEND _image_formats "exr")
endif()
foreach(_image_format IN LISTS _image_formats)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/image.inl.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${_image_format}.inl"
    @ONLY)
endforeach()

f3d_plugin_declare_reader(
  NAME PNG
  EXTENSIONS png
  MIMETYPES image/png
  VTK_IMPORTER vtkF3DImageImporter
  FORMAT_DESCRIPTION "PNG Image"
  EXCLUDE_FROM_THUMBNAILER
  ${_SUPPORTS_STREAM}
  CAN_READ CUSTOM
  CUSTOM_CODE "${CMAKE_CURRENT_BINARY_DIR}/png.inl"
)

f3d_plugin_declare_reader(
  NAME JPEG
  EXTENSIONS jpg jpeg
  MIMETYPES image/jpeg
  VTK_IMPORTER vtkF3DImageImporter
  FORMAT_DESCRIPTION "JPEG Image"
  EXCLUDE_FROM_THUMBNAILER
  ${_SUPPORTS_STREAM}
  CAN_READ CUSTOM
  CUSTOM_CODE "${CMAKE_CURRENT_BINARY_DIR}/jpeg.inl"
)

f3d_plugin_declare_reader(
  NAME BMP
  EXTENSIONS bmp
  MIMETYPES image/bmp
  VTK_IMPORTER vtkF3DImageImporter
  FORMAT_DESCRIPTION "BMP Image"
  EXCLUDE_FROM_THUMBNAILER
  ${_SUPPORTS_STREAM}
  CAN_READ CUSTOM
  CUSTOM_CODE "${CMAKE_CURRENT_BINARY_DIR}/bmp.inl"
)

f3d_plugin_declare_reader(
  NAME TGA
  EXTENSIONS tga
  MIMETYPES image/x-tga
  VTK_IMPORTER vtkF3DImageImporter
  FORMAT_DESCRIPTION "TGA Image"
  ${_SUPPORTS_STREAM}
  CAN_READ CUSTOM
  CUSTOM_CODE "${CMAKE_CURRENT_BINARY_DIR}/tga.inl"
)

f3d_plugin_declare_reader(
  NAME HDR
  EXTENSIONS hdr pic
  MIMETYPES image/vnd.radiance
  VTK_IMPORTER vtkF3DImageImporter
  FORMAT_DESCRIPTION "HDR Radiance Image"
  ${_SUPPORTS_STREAM}
  CAN_READ CUSTOM
  CUSTOM_CODE "${CMAKE_CURRENT_BINARY_DIR}/hdr.inl"
)

if(F3D_MODULE_WEBP)
  f3d_plugin_declare_reader(
    NAME WebP
    EXTENSIONS webp
    MIMETYPES image/webp
    VTK_IMPORTER vtkF3DImageImporter
    FORMAT_DESCRIPTION "WebP Image"
    EXCLUDE_FROM_THUMBNAILER
    ${_SUPPORTS_STREAM}
    CAN_READ CUSTOM
    CUSTOM_CODE "${CMAKE_CURRENT_BINARY_DIR}/webp.inl"
  )
endif()

if(F3D_MODULE_EXR)
  f3d_plugin_declare_reader(
    NAME EXR
    EXTENSIONS exr
    MIMETYPES image/x-exr
    VTK_IMPORTER vtkF3DImageImporter
    FORMAT_DESCRIPTION "OpenEXR Image"
    ${_SUPPORTS_STREAM}
    CAN_READ CUSTOM
    CUSTOM_CODE "${CMAKE_CURRENT_BINARY_DIR}/exr.inl"
  )
endif()

f3d_plugin_declare_reader(
  NAME VRMLReader
  EXTENSIONS wrl vrml
  MIMETYPES model/vrml
  VTK_IMPORTER vtkVRMLImporter
  FORMAT_DESCRIPTION "VRML"
)

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.20260106)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME VTKLegacy
  EXTENSIONS vtk
  MIMETYPES application/vnd.vtk
  VTK_READER vtkDataSetReader
  FORMAT_DESCRIPTION "VTK Legacy"
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/vtk.inl"
)

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.6.20260106)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME VTKXMLVTU
  EXTENSIONS vtu
  MIMETYPES application/vnd.vtu
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML UnstructuredGrid"
  ${_SUPPORTS_STREAM}
  CAN_READ MEMBER
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/xml.inl"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTP
  EXTENSIONS vtp
  MIMETYPES application/vnd.vtp
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML PolyData"
  ${_SUPPORTS_STREAM}
  CAN_READ MEMBER
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/xml.inl"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTI
  EXTENSIONS vti
  MIMETYPES application/vnd.vti
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML ImageData"
  ${_SUPPORTS_STREAM}
  CAN_READ MEMBER
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/xml.inl"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTR
  EXTENSIONS vtr
  MIMETYPES application/vnd.vtr
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML RectangularGrid"
  ${_SUPPORTS_STREAM}
  CAN_READ MEMBER
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/xml.inl"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTS
  EXTENSIONS vts
  MIMETYPES application/vnd.vts
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML StructuredGrid"
  ${_SUPPORTS_STREAM}
  CAN_READ MEMBER
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/xml.inl"
)

f3d_plugin_declare_reader(
  NAME VTKXMLVTM
  EXTENSIONS vtm
  MIMETYPES application/vnd.vtm
  VTK_READER vtkXMLGenericDataObjectReader
  FORMAT_DESCRIPTION "VTK XML MultiBlock"
)

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250501)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME QuakeMDL
  EXTENSIONS mdl
  MIMETYPES application/vnd.mdl
  OPTIONS skin_index
  VTK_IMPORTER vtkF3DQuakeMDLImporter
  FORMAT_DESCRIPTION "Quake 1 MDL model"
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/mdl.inl"
)

f3d_plugin_declare_reader(
  NAME SPZ
  EXTENSIONS spz
  MIMETYPES application/vnd.spz
  VTK_READER vtkF3DSPZReader
  FORMAT_DESCRIPTION "Compressed 3D gaussian splats"
  SCORE 40 # CanReadFile is just a gunzip check
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
)

f3d_plugin_declare_reader(
  NAME Splat
  EXTENSIONS splat
  MIMETYPES application/vnd.splat
  VTK_READER vtkF3DSplatReader
  FORMAT_DESCRIPTION "3D Gaussian splats"
  SCORE 40 # Any random correctly sized file is a false positive
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
)

f3d_plugin_declare_reader(
  NAME PLYReader
  EXTENSIONS ply
  MIMETYPES application/vnd.ply
  VTK_READER vtkF3DPLYReader
  FORMAT_DESCRIPTION "Polygon"
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/ply.inl"
)

f3d_plugin_build(
  NAME native
  VERSION 1.0
  DESCRIPTION "Native VTK I/O support"
  VTK_MODULES IOXML IOLegacy IOCityGML IOImage IOGeometry IOPLY
  MIMETYPE_XML_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/f3d-3d-formats.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/f3d-3d-image-formats.xml"
    "${CMAKE_CURRENT_SOURCE_DIR}/f3d-vtk-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
  FORCE_STATIC
)
