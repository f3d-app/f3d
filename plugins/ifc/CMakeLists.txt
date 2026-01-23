cmake_minimum_required(VERSION 3.21)

project(f3d-plugin-ifc)

include(GNUInstallDirs)

if(PROJECT_IS_TOP_LEVEL)
  find_package(f3d REQUIRED COMPONENTS pluginsdk)
else()
  include(f3dPlugin)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(web-ifc REQUIRED)

message(STATUS "Plugin: web-ifc found")

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME IFC
  EXTENSIONS ifc
  MIMETYPES application/x-ifc
  VTK_READER vtkF3DIFCReader
  FORMAT_DESCRIPTION "Industry Foundation Classes"
  SUPPORTS_STREAM
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/IFC.inl"
  OPTIONS circle_segments read_openings read_spaces
)

f3d_plugin_build(
  NAME ifc
  VERSION 1.0
  DESCRIPTION "IFC/BIM support via web-ifc"
  VTK_MODULES CommonDataModel
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-ifc-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)
