cmake_minimum_required(VERSION 3.19)

project(f3d-plugin-alembic)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  find_package(f3d REQUIRED)
else()
  include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)
endif()

find_package(Alembic 1.7 REQUIRED)

message(STATUS "Plugin: Alembic ${Alembic_VERSION} found")

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME Alembic
  EXTENSIONS abc
  MIMETYPES application/vnd.abc
  VTK_IMPORTER vtkF3DAlembicImporter
  FORMAT_DESCRIPTION "Alembic"
)

set(rpaths "")
get_target_property(target_type Alembic::Alembic TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
  list(APPEND rpaths "$<TARGET_FILE_DIR:Alembic::Alembic>")
endif ()
get_target_property(target_type Imath::Imath TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
  list(APPEND rpaths "$<TARGET_FILE_DIR:Imath::Imath>")
endif ()

f3d_plugin_build(
  NAME alembic
  VERSION 1.0
  DESCRIPTION "Alembic support (version ${Alembic_VERSION})"
  VTK_MODULES CommonCore CommonExecutionModel IOImport
  ADDITIONAL_RPATHS ${rpaths}
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-alembic-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)
