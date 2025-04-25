cmake_minimum_required(VERSION 3.21)

project(f3d-plugin-spz)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(PROJECT_IS_TOP_LEVEL)
  find_package(f3d REQUIRED COMPONENTS pluginsdk)
else()
  include(f3dPlugin)
endif()

# TODO: can we get rid of this?
find_package(ZLIB REQUIRED)

find_package(spz 1.1 REQUIRED)

message(STATUS "Plugin: spz ${spz_VERSION} found")

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME spz
  EXTENSIONS spz
  MIMETYPES application/vnd.spz
  VTK_READER vtkF3DSPZReader
  FORMAT_DESCRIPTION "compressed 3D gaussians"
)

set(rpaths "")
get_target_property(target_type spz::spz TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
  list(APPEND rpaths "$<TARGET_FILE_DIR:spz::spz>")
endif ()

f3d_plugin_build(
  NAME spz
  VERSION 1.0
  DESCRIPTION "spz support (version ${spz_VERSION})"
  VTK_MODULES IOImage
  ADDITIONAL_RPATHS ${rpaths}
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-spz-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)
