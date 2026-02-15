cmake_minimum_required(VERSION 3.21)

project(f3d-plugin-draco)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(PROJECT_IS_TOP_LEVEL)
  find_package(f3d REQUIRED COMPONENTS pluginsdk)
else()
  include(f3dPlugin)
endif()

find_package(draco 1.5.6 REQUIRED)

message(STATUS "Plugin: draco ${draco_VERSION} found")

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME Draco
  EXTENSIONS drc
  MIMETYPES application/vnd.drc
  VTK_READER vtkF3DDracoReader
  SUPPORTS_STREAM
  STANDARD_CAN_READ
  FORMAT_DESCRIPTION "Draco"
)

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10884
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240214)

  set(_SUPPORTS_STREAM)
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20250923)
    set(_SUPPORTS_STREAM SUPPORTS_STREAM)
  endif()

  set(_GLTF_SCORE 51) # GLTFDraco should be lower than GLTF
  if(VTK_VERSION VERSION_LESS 9.6.20260128)
    set(_GLTF_SCORE 70) # Except with older VTK version where CanReadFile was not implemented
  endif()

  f3d_plugin_declare_reader(
    NAME GLTFDraco
    EXTENSIONS gltf
    MIMETYPES model/gltf+json
    VTK_IMPORTER vtkF3DGLTFDracoImporter
    FORMAT_DESCRIPTION "GL Transmission Format"
    SCORE ${_GLTF_SCORE}
    ${_SUPPORTS_STREAM}
    STANDARD_CAN_READ
  )

  # GLB reader will be removed once VTK v9.6 support is dropped
  f3d_plugin_declare_reader(
    NAME GLBDraco
    EXTENSIONS glb
    MIMETYPES model/gltf-binary
    VTK_IMPORTER vtkF3DGLTFDracoImporter
    FORMAT_DESCRIPTION "GL Transmission Format (binary)"
    SCORE ${_GLTF_SCORE}
    ${_SUPPORTS_STREAM}
    STANDARD_CAN_READ
    CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/glb.inl"
  )
endif()

set(rpaths "")
get_target_property(target_type draco::draco TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
  list(APPEND rpaths "$<TARGET_FILE_DIR:draco::draco>")
endif ()

f3d_plugin_build(
  NAME draco
  VERSION 1.0
  DESCRIPTION "Draco support (version ${draco_VERSION})"
  ADDITIONAL_RPATHS ${rpaths}
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-draco-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)
