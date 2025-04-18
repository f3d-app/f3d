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

if(BUILD_TESTING)
  enable_testing()

  # Finding f3d is needed to test the plugin
  find_package(f3d REQUIRED COMPONENTS application)
  add_test(NAME TestPluginExample
          COMMAND "$<TARGET_FILE:f3d::f3d>"
                  "--no-config"
                  "--no-render"
                  "--verbose"
                  "--load-plugins=$<TARGET_FILE:f3d-plugin-example>"
                  "${CMAKE_CURRENT_SOURCE_DIR}/data.expl"
  )

  set_tests_properties(TestPluginExample PROPERTIES
    PASS_REGULAR_EXPRESSION "Number of points: 3"
  )
endif()
