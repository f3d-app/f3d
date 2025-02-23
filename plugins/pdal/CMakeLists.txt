cmake_minimum_required(VERSION 3.21)

project(f3d-plugin-pdal)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(PROJECT_IS_TOP_LEVEL)
  find_package(f3d REQUIRED COMPONENTS pluginsdk)
else()
  include(f3dPlugin)
endif()

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME LAS
  EXTENSIONS las
  MIMETYPES application/vnd.las
  VTK_READER vtkPDALReader
  FORMAT_DESCRIPTION "LAS"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/pdal.inl"
)

f3d_plugin_build(
  NAME pdal
  VERSION 1.0
  DESCRIPTION "VTK PDAL support"
  VTK_MODULES IOPDAL
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-pdal-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)
