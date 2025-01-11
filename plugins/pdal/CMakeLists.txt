cmake_minimum_required(VERSION 3.19)

project(f3d-plugin-pdal)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  find_package(f3d REQUIRED COMPONENTS pluginsdk)
else()
  include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)
endif()

message(STATUS "[Gapry][PoC] Plugin: PDAL found")

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME PDAL
  EXTENSIONS bpf matlab npy pgpointcloud tiledb
  MIMETYPES
  VTK_IMPORTER vtkF3DPDALReader
  FORMAT_DESCRIPTION "Point Cloud Data"
)

f3d_plugin_build(
  NAME pdal
  VERSION 1.0
  DESCRIPTION "PDAL support"
  VTK_MODULES IOPDAL
  FREEDESKTOP
)

