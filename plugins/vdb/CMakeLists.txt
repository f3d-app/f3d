cmake_minimum_required(VERSION 3.21)

project(f3d-plugin-vdb)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(PROJECT_IS_TOP_LEVEL)
  find_package(f3d REQUIRED)
else()
  include(f3dPlugin)
endif()

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME VDB
  EXTENSIONS vdb
  MIMETYPES application/vnd.vdb
  VTK_READER vtkOpenVDBReader
  FORMAT_DESCRIPTION "VDB"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/vdb.inl"
)

f3d_plugin_build(
  NAME vdb
  VERSION 1.0
  DESCRIPTION "VTK OpenVDB support"
  VTK_MODULES IOOpenVDB
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-vdb-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)
