cmake_minimum_required(VERSION 3.21)

project(f3d-plugin-assimp)

include(GNUInstallDirs)

# Check if the plugin is built externally
if(PROJECT_IS_TOP_LEVEL)
  find_package(f3d REQUIRED COMPONENTS pluginsdk)
else()
  include(f3dPlugin)
endif()

# We cannot use the CMake version check directly because of an assimp issue
# https://github.com/assimp/assimp/issues/6204#issuecomment-2931440948
find_package(assimp REQUIRED)

if("${assimp_VERSION}" VERSION_LESS "5.4")
  message(FATAL_ERROR "assimp plugin is only compatible with assimp >= 5.4")
endif()

set(F3D_ASSIMP_VERSION "${assimp_VERSION}" CACHE INTERNAL "")
message(STATUS "Plugin: assimp ${assimp_VERSION} found")

f3d_plugin_init()

set(_SUPPORTS_STREAM)
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251016)
  set(_SUPPORTS_STREAM SUPPORTS_STREAM)
endif()

f3d_plugin_declare_reader(
  NAME FBX
  EXTENSIONS fbx
  MIMETYPES application/vnd.fbx
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "Filmbox"
  ${_SUPPORTS_STREAM}
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/fbx.inl"
)

f3d_plugin_declare_reader(
  NAME COLLADA
  EXTENSIONS dae
  MIMETYPES application/vnd.dae
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "COLLADA"
  ${_SUPPORTS_STREAM}
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/collada.inl"
)

f3d_plugin_declare_reader(
  NAME DXF
  EXTENSIONS dxf
  MIMETYPES image/vnd.dxf
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "AutoCAD DXF"
  ${_SUPPORTS_STREAM}
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/dxf.inl"
)

f3d_plugin_declare_reader(
  NAME OFF
  EXTENSIONS off
  MIMETYPES application/vnd.off
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "Object File Format"
  ${_SUPPORTS_STREAM}
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/off.inl"
)

f3d_plugin_declare_reader(
  NAME DirectX
  EXTENSIONS x
  MIMETYPES application/vnd.x
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "DirectX File Format"
  ${_SUPPORTS_STREAM}
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/x.inl"
)

f3d_plugin_declare_reader(
  NAME 3MF
  EXTENSIONS 3mf
  MIMETYPES model/3mf
  VTK_IMPORTER vtkF3DAssimpImporter
  FORMAT_DESCRIPTION "3D Manufacturing Format"
  ${_SUPPORTS_STREAM}
  CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/3mf.inl"
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
  VTK_MODULES IOImage RenderingOpenGL2
  ADDITIONAL_RPATHS ${rpaths}
  MIMETYPE_XML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/f3d-assimp-formats.xml"
  CONFIGURATION_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/configs/config.d" "${CMAKE_CURRENT_SOURCE_DIR}/configs/thumbnail.d"
  FREEDESKTOP
)
