cmake_minimum_required(VERSION 3.19)

project(example-plugin)

include(GNUInstallDirs)

find_package(f3d REQUIRED COMPONENTS pluginsdk)

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME ExampleReader
  EXTENSIONS expl
  MIMETYPES application/vnd.expl
  VTK_READER vtkF3DExampleReader
  FORMAT_DESCRIPTION "Example reader"
)

f3d_plugin_build(
  NAME example
  VERSION 1.0
  DESCRIPTION "Example plugin"
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-example-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)

option(BUILD_TESTING "Build the example tests" OFF)
if(BUILD_TESTING)
  enable_testing()
  f3d_test(NAME TestPluginExample DATA ${CMAKE_CURRENT_SOURCE_DIR}/data.expl NO_RENDER ARGS --verbose --load-plugins=$<TARGET_FILE:f3d-plugin-example> REGEXP "Number of points: 3")
endif()
