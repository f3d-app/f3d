include(${CMAKE_SOURCE_DIR}/cmake/f3dPlugin.cmake)

find_package(assimp 5.0 REQUIRED)

set(F3D_ASSIMP_VERSION "${assimp_VERSION}" CACHE INTERNAL "")
message(STATUS "Plugin: assimp ${assimp_VERSION} found")

if("${assimp_VERSION}" VERSION_GREATER_EQUAL "5.1.0")
  message(WARNING "Plugin: assimp: Complex animations are not working with assimp 5.1.0 and newer, use assimp 5.0.X for animation support with assimp formats.")
endif()
if("${assimp_VERSION}" VERSION_LESS "5.1.0")
  message(WARNING "Plugin: assimp: Embedded texture are only working with assimp 5.1.X and newer.")
endif()

f3d_plugin_init()

f3d_plugin_declare_reader(
  NAME FBX
  EXTENSIONS fbx
  MIMETYPES application/vnd.fbx
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "Filmbox"
)

f3d_plugin_declare_reader(
  NAME Collada
  EXTENSIONS dae
  MIMETYPES application/vnd.dae
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "Collada"
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/collada.inl"
)

f3d_plugin_declare_reader(
  NAME DXF
  EXTENSIONS dxf
  MIMETYPES image/vnd.dxf
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "AutoCAD DXF"
)

f3d_plugin_declare_reader(
  NAME OFF
  EXTENSIONS off
  MIMETYPES application/vnd.off
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "Object File Format"
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
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-assimp-formats.xml"
  FREEDESKTOP
)
