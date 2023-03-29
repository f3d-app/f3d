cmake_minimum_required(VERSION 3.19)

project(f3d-plugin-occt)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  find_package(f3d REQUIRED)
else()
  include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)
endif()

find_package(OpenCASCADE REQUIRED)

set(F3D_PLUGIN_OCCT_XCAF 0)

if("${OpenCASCADE_VERSION}" VERSION_LESS "7.5.0")
  message(FATAL_ERROR "Plugin: OpenCASCADE: ${OpenCASCADE_VERSION} is not supported by F3D, please update your OpenCASCADE installation.")
endif()

if((NOT TARGET "TKSTEP") AND (NOT TARGET "TKIGES") AND (NOT TARGET "TKMesh"))
    message(FATAL_ERROR "Plugin: OpenCASCADE does not contain required modules")
endif()

message(STATUS "Plugin: OpenCASCADE ${OpenCASCADE_VERSION} found")

if((TARGET "TKXDESTEP") AND (TARGET "TKXDEIGES"))
    set(F3D_PLUGIN_OCCT_XCAF 1)
else()
    message(WARNING "OpenCASCADE: TKXDESTEP and TKXDEIGES are not found. Parts color and name are not supported.")
endif()

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME STEP
  EXTENSIONS stp step stpnc p21 210
  MIMETYPES application/vnd.step
  VTK_READER vtkF3DOCCTReader
  FORMAT_DESCRIPTION "STEP ISO 10303"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/step.inl"
)

f3d_plugin_declare_reader(
  NAME IGES
  EXTENSIONS igs iges
  MIMETYPES model/iges
  VTK_READER vtkF3DOCCTReader
  FORMAT_DESCRIPTION "Initial Graphics Exchange Specification"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/iges.inl"
)

set(rpaths "")
get_target_property(target_type TKernel TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
  list(APPEND rpaths "${OpenCASCADE_LIBRARY_DIR}")
endif ()

f3d_plugin_build(
  NAME occt
  VERSION 1.0
  DESCRIPTION "OpenCASCADE support (version ${OpenCASCADE_VERSION})"
  VTK_MODULES CommonCore CommonExecutionModel IOImport
  ADDITIONAL_RPATHS ${rpaths}
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-occt-formats.xml"
  FREEDESKTOP
)
