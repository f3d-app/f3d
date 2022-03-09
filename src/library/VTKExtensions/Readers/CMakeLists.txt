# This macro generates the reader header from the F3DReaderBoilerPlate.h.in template file
# It also adds the code in the F3DReaderInstanciator.h.in file
macro(declare_f3d_reader)
  cmake_parse_arguments(F3D_READER "HAS_CUSTOM_CODE" "NAME;VTK_IMPORTER;VTK_READER;DESCRIPTION" "EXTENSIONS;MIMETYPES" ${ARGN})

  if(F3D_READER_HAS_CUSTOM_CODE)
    set(F3D_READER_CUSTOM_CODE 1)
  else()
    set(F3D_READER_CUSTOM_CODE 0)
  endif()

  list(APPEND F3D_READER_FILES "F3D${F3D_READER_NAME}Reader.h")
  set(F3D_READER_INCLUDES_CODE
    "${F3D_READER_INCLUDES_CODE}#include \"F3D${F3D_READER_NAME}Reader.h\"\n")
  set(F3D_READER_INSTANCES_CODE
    "${F3D_READER_INSTANCES_CODE}  F3D${F3D_READER_NAME}Reader InstanceOf${F3D_READER_NAME}Reader;\n")
  set(F3D_READER_REGISTER_CODE
    "${F3D_READER_REGISTER_CODE}    F3DReaderFactory::Register(&InstanceOf${F3D_READER_NAME}Reader);\n")
  set(F3D_READER_UNREGISTER_CODE
    "${F3D_READER_UNREGISTER_CODE}    F3DReaderFactory::Unregister(&InstanceOf${F3D_READER_NAME}Reader);\n")

  list(APPEND F3D_MIME_TYPES "${F3D_READER_MIMETYPES}")
  list(APPEND F3D_EXTENSIONS "${F3D_READER_EXTENSIONS}")

  if(WIN32)
    # used for the NSIS file association during installation (see packaging.cmake)
    set(F3D_FILE_ASSOCIATION_READER "${F3D_READER_EXTENSIONS}")
    list(TRANSFORM F3D_FILE_ASSOCIATION_READER PREPEND "'.")
    list(TRANSFORM F3D_FILE_ASSOCIATION_READER APPEND "' '${F3D_READER_DESCRIPTION}'")
    list(APPEND F3D_FILE_ASSOCIATION_LIST "${F3D_FILE_ASSOCIATION_READER}")
  endif()

  list(TRANSFORM F3D_READER_EXTENSIONS PREPEND "\".")
  list(TRANSFORM F3D_READER_EXTENSIONS APPEND "\"")
  list(JOIN F3D_READER_EXTENSIONS ", " F3D_READER_EXTENSIONS)

  list(TRANSFORM F3D_READER_MIMETYPES PREPEND "\"")
  list(TRANSFORM F3D_READER_MIMETYPES APPEND "\"")
  list(JOIN F3D_READER_MIMETYPES ", " F3D_READER_MIMETYPES)

  if(F3D_READER_VTK_IMPORTER)
    set(F3D_READER_HAS_SCENE_READER 1)
  else()
    set(F3D_READER_HAS_SCENE_READER 0)
  endif()

  if(F3D_READER_VTK_READER)
    set(F3D_READER_HAS_GEOMETRY_READER 1)
  else()
    set(F3D_READER_HAS_GEOMETRY_READER 0)
  endif()

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/F3DReaderBoilerPlate.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/F3D${F3D_READER_NAME}Reader.h")
endmacro()

declare_f3d_reader(
  NAME 3DS
  EXTENSIONS 3ds
  MIMETYPES application/vnd.3ds
  VTK_IMPORTER vtk3DSImporter
  DESCRIPTION "3DS files reader"
)

declare_f3d_reader(
  NAME CityGML
  EXTENSIONS gml
  MIMETYPES application/gml+xml
  VTK_READER vtkCityGMLReader
  DESCRIPTION "CityGML files reader"
)

declare_f3d_reader(
  NAME DICOM
  EXTENSIONS dcm
  MIMETYPES application/dicom
  VTK_READER vtkDICOMImageReader
  DESCRIPTION "DICOM images files reader"
)

declare_f3d_reader(
  NAME GLTF
  EXTENSIONS gltf glb
  MIMETYPES model/gltf-binary model/gltf+json
  VTK_READER vtkGLTFReader
  VTK_IMPORTER vtkGLTFImporter
  DESCRIPTION "glTF files reader"
  HAS_CUSTOM_CODE
)

declare_f3d_reader(
  NAME MetaImage
  EXTENSIONS mha mhd
  MIMETYPES application/vnd.mhd
  VTK_READER vtkMetaImageReader
  DESCRIPTION "MetaImage files reader"
)

declare_f3d_reader(
  NAME Nrrd
  EXTENSIONS nrrd nhdr
  MIMETYPES application/vnd.nrrd
  VTK_READER vtkNrrdReader
  DESCRIPTION "Nrrd files reader"
)

declare_f3d_reader(
  NAME OBJ
  EXTENSIONS obj
  MIMETYPES model/obj
  VTK_READER vtkOBJReader
  VTK_IMPORTER vtkOBJImporter
  DESCRIPTION "OBJ files reader"
  HAS_CUSTOM_CODE
)

declare_f3d_reader(
  NAME PLYReader
  EXTENSIONS ply
  MIMETYPES application/vnd.ply
  VTK_READER vtkPLYReader
  DESCRIPTION "PLY files reader"
)

declare_f3d_reader(
  NAME PTS
  EXTENSIONS pts
  MIMETYPES application/vnd.pts
  VTK_READER vtkPTSReader
  DESCRIPTION "PTS files reader"
)

declare_f3d_reader(
  NAME STL
  EXTENSIONS stl
  MIMETYPES model/stl
  VTK_READER vtkSTLReader
  DESCRIPTION "STL files reader"
  HAS_CUSTOM_CODE
)

declare_f3d_reader(
  NAME TIFF
  EXTENSIONS tiff tif
  MIMETYPES application/x-tgif
  VTK_READER vtkTIFFReader
  DESCRIPTION "TIFF files reader"
)

declare_f3d_reader(
  NAME VRMLReader
  EXTENSIONS wrl vmrl
  MIMETYPES model/vrml
  VTK_IMPORTER vtkVRMLImporter
  DESCRIPTION "VRML files reader"
)

declare_f3d_reader(
  NAME VTKLegacy
  EXTENSIONS vtk
  MIMETYPES application/vnd.vtk
  VTK_READER vtkPDataSetReader
  DESCRIPTION "VTK Legacy files reader"
)

declare_f3d_reader(
  NAME VTKXML
  EXTENSIONS vtp vtu vtr vti vts vtm
  MIMETYPES application/vnd.vtp application/vnd.vtu application/vnd.vtr application/vnd.vti application/vnd.vts application/vnd.vtm
  VTK_READER vtkXMLGenericDataObjectReader
  DESCRIPTION "VTK XML files reader"
)

if(F3D_MODULE_OCCT)
  declare_f3d_reader(
    NAME OCCT
    EXTENSIONS stp step igs iges
    MIMETYPES model/iges application/vnd.step
    VTK_READER vtkF3DOCCTReader
    DESCRIPTION "OpenCascade files reader"
    HAS_CUSTOM_CODE
  )
endif()

if(F3D_MODULE_ASSIMP)
  declare_f3d_reader(
    NAME Assimp
    EXTENSIONS fbx dae dxf off
    MIMETYPES application/vnd.fbx application/vnd.dae image/vnd.dxf application/vnd.off
    VTK_IMPORTER vtkF3DAssimpImporter
    DESCRIPTION "Assimp files reader"
  )
endif()

if(F3D_MODULE_ALEMBIC)
  declare_f3d_reader(
    NAME Alembic
    EXTENSIONS abc
    MIMETYPES application/vnd.abc
    VTK_IMPORTER vtkF3DAlembicImporter
    DESCRIPTION "Alembic files reader"
  )
endif()

if(F3D_MODULE_EXODUS)
  declare_f3d_reader(
    NAME ExodusII
    EXTENSIONS exo ex2 e
    MIMETYPES application/vnd.exodus
    VTK_READER vtkExodusIIReader
    DESCRIPTION "ExodusII files reader"
    HAS_CUSTOM_CODE
  )
endif()

# Generate the code of the F3DReaderInstantiator class
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/F3DReaderInstantiator.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/F3DReaderInstantiator.h"
  @ONLY)

unset(F3D_SUPPORTED_MIME_TYPES CACHE)
unset(F3D_SUPPORTED_EXTENSIONS CACHE)

set(F3D_SUPPORTED_MIME_TYPES "${F3D_MIME_TYPES}" CACHE INTERNAL "Supported mime-type list" FORCE)
set(F3D_SUPPORTED_EXTENSIONS "${F3D_EXTENSIONS}" CACHE INTERNAL "Supported extension list" FORCE)

if(WIN32)
  unset(F3D_FILE_ASSOCIATION_NSIS CACHE)
  set(F3D_FILE_ASSOCIATION_NSIS "${F3D_FILE_ASSOCIATION_LIST}" CACHE INTERNAL "File association list used by NSIS" FORCE)
endif()

list(LENGTH F3D_READER_FILES F3D_NB_READERS)
message(STATUS "${F3D_NB_READERS} readers found.")
message(STATUS "Supported extensions: ${F3D_SUPPORTED_EXTENSIONS}")

set(MODULES_SPECIFIC_INCLUDE_DIRS "")
set(MODULES_SPECIFIC_LIBRARIES "")

set(classes
  vtkF3DGenericImporter
  vtkF3DPostProcessFilter
  vtkF3DMetaReader
  F3DReader
  F3DReaderFactory
  )

if(F3D_MODULE_ASSIMP)
  list(APPEND classes vtkF3DAssimpImporter)

  # Note: Assimp CMake module file does not declare Debug target properties, just map it to Release properties
  get_target_property(F3D_ASSIMP_DEBUG_VAR assimp::assimp IMPORTED_IMPLIB_RELEASE)
  if(NOT F3D_ASSIMP_DEBUG_VAR)
    set_target_properties(assimp::assimp PROPERTIES MAP_IMPORTED_CONFIG_DEBUG Release)
  endif()
endif()

if(F3D_MODULE_OCCT)
  list(APPEND classes vtkF3DOCCTReader)
  # Note: OpenCASCADE does not declare the libs in the correct dependency order.
  # This could be an issue on Linux when linking with static libraries.
  # See .github/workflows/ci.yml for a scripted fix
endif()

if(F3D_MODULE_ALEMBIC)
  list(APPEND classes vtkF3DAlembicImporter)
endif()

vtk_module_add_module(f3d::VTKExtensionsReaders
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_include(f3d::VTKExtensionsReaders
  PRIVATE
    $<BUILD_INTERFACE:${OpenCASCADE_INCLUDE_DIR}>)

vtk_module_link(f3d::VTKExtensionsReaders
  PRIVATE
    ${OpenCASCADE_LIBRARIES})
