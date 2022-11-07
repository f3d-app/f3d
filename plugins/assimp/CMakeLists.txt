include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)

find_package(assimp 5.0 REQUIRED)

set(F3D_ASSIMP_VERSION "${assimp_VERSION}" CACHE INTERNAL "")
message(STATUS "Plugin: assimp ${assimp_VERSION} found")

if("${assimp_VERSION}" VERSION_GREATER_EQUAL "5.1.0")
  message(WARNING "Plugin: assimp: Animations are not working with assimp 5.1.0 and newer, use assimp 5.0.X for animation support with assimp formats.")
endif()
if("${assimp_VERSION}" VERSION_LESS "5.1.0")
  message(WARNING "Plugin: assimp: Embedded texture are only working with assimp 5.1.X and newer.")
endif()

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME Assimp
  EXTENSIONS fbx dae dxf off
  MIMETYPES application/vnd.fbx application/vnd.dae image/vnd.dxf application/vnd.off
  VTK_IMPORTER vtkF3DAssimpImporter
  DESCRIPTION "Assimp files reader"
)

set(rpaths "")
get_target_property(target_type assimp::assimp TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
  list(APPEND rpaths "$<TARGET_FILE_DIR:assimp::assimp>")
endif ()

f3d_plugin_build(
  NAME assimp
  VERSION 1.0
  DESCRIPTION "Assimp support (version ${assimp_VERSION})"
  VTK_MODULES CommonCore CommonExecutionModel IOImport
  ADDITIONAL_RPATHS ${rpaths}
)
