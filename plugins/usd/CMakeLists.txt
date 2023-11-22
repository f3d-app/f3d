cmake_minimum_required(VERSION 3.19)

project(f3d-plugin-usd)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  find_package(f3d REQUIRED)
else()
  include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)
endif()

find_package(pxr REQUIRED)

message(STATUS "Plugin: USD found")

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME USD
  EXTENSIONS usd usdc usda usdz
  MIMETYPES application/vnd.usd application/vnd.usdc model/vnd.usda model/vnd.usdz+zip
  VTK_IMPORTER vtkF3DUSDImporter
  FORMAT_DESCRIPTION "Universal Scene Descriptor"
)

set(rpaths "")
get_target_property(target_type usdGeom TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
  list(APPEND rpaths "$<TARGET_FILE_DIR:usdGeom>")
endif ()

f3d_plugin_build(
  NAME usd
  VERSION 1.0
  DESCRIPTION "USD support"
  VTK_MODULES IOImage
  ADDITIONAL_RPATHS ${rpaths}
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-usd-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)
