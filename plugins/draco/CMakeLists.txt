include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)

find_package(draco REQUIRED)

message(STATUS "Plugin: draco ${draco_VERSION} found")

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME Draco
  EXTENSIONS drc
  MIMETYPES application/vnd.drc
  VTK_READER vtkF3DDracoReader
  FORMAT_DESCRIPTION "Draco"
)

set(rpaths "")
get_target_property(target_type draco::draco TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
  list(APPEND rpaths "$<TARGET_FILE_DIR:draco::draco>")
endif ()

f3d_plugin_build(
  NAME draco
  VERSION 1.0
  DESCRIPTION "Draco support (version ${draco_VERSION})"
  VTK_MODULES CommonCore CommonExecutionModel
  ADDITIONAL_RPATHS ${rpaths}
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-draco-formats.xml"
  FREEDESKTOP
)
