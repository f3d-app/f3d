cmake_minimum_required(VERSION 3.19)

project(f3d-plugin-usd)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  find_package(f3d REQUIRED COMPONENTS pluginsdk)
else()
  include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)
endif()

find_package(pxr REQUIRED)

if("${PXR_VERSION}" VERSION_LESS "2408")
  message(FATAL_ERROR "Plugin: USD: ${PXR_VERSION} is not supported by F3D, please update your USD installation.")
endif()

message(STATUS "Plugin: USD ${PXR_VERSION} found")

f3d_plugin_init()

set(_SUPPORTS_STREAM)
set(_has_supports_stream 0)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251016)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
  set(_has_supports_stream 1)
endif()

f3d_plugin_declare_reader(
  NAME USD
  EXTENSIONS usd usdc usda usdz
  MIMETYPES application/vnd.usd application/vnd.usdc model/vnd.usda model/vnd.usdz+zip
  VTK_IMPORTER vtkF3DUSDImporter
  FORMAT_DESCRIPTION "Universal Scene Descriptor"
  ${_SUPPORTS_STREAM}
  CAN_READ STATIC
)

set(_additional_rpath "")
list(GET PXR_LIBRARIES 0 _first_lib)
get_target_property(_target_type ${_first_lib} TYPE)
if(_target_type STREQUAL SHARED_LIBRARY)
  list(APPEND _additional_rpath "$<TARGET_FILE_DIR:${_first_lib}>")
endif ()

f3d_plugin_build(
  NAME usd
  VERSION 1.0
  DESCRIPTION "USD support"
  VTK_MODULES IOImage
  ADDITIONAL_RPATHS ${_additional_rpath}
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-usd-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)

# TODO: improve f3d_plugin_build to include that

# installation
install(
  FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/plugInfo.json"
  DESTINATION "${F3D_PLUGINS_INSTALL_DIR}/lib/usd/f3d/resources"
  COMPONENT plugin
)

# build environment
file(MAKE_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/usd/f3d/resources")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/resources/plugInfo.json"
  "${CMAKE_BINARY_DIR}/lib/usd/f3d/resources/plugInfo.json"
  COPYONLY
)
