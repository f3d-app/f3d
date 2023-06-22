#[==[
@file f3dPlugin.cmake

This module contains all the macros required to define a plugin to extend
the reading capabilities of F3D.
#]==]

set(_f3dPlugin_dir "${CMAKE_CURRENT_LIST_DIR}")

#[==[
@brief Initialize the plugin.

~~~
f3d_plugin_init()
~~~

This initializes the variable used internally to accumulate the reader declarations.
#]==]

macro(f3d_plugin_init)
  set(F3D_PLUGIN_INCLUDES_CODE "")
  set(F3D_PLUGIN_REGISTER_CODE "")
  set(F3D_PLUGIN_MIMETYPES "")
  set(F3D_PLUGIN_MIMETYPES_THUMB "")
  set(F3D_PLUGIN_CURRENT_READER_INDEX 0)
  set(F3D_PLUGIN_JSON "{ \"readers\": [] }")
endmacro()

#[==[
@brief Declare a reader.

~~~
f3d_plugin_declare_reader(
  NAME                   <string>
  [VTK_IMPORTER          <class>]
  [VTK_READER            <class>]
  [FORMAT_DESCRIPTION    <string>]
  [SCORE                 <integer>]
  [EXCLUDE_FROM_THUMBNAILER]
  [CUSTOM_CODE           <file>]
  EXTENSIONS             <string>...
  MIMETYPES              <string>...)
~~~

Declare a reader. Can be called several times and must be called after ``f3d_plugin_init``.

The `NAME` argument is required. The arguments are as follows:

  * `NAME`: (Required) The name of the reader.
  * `VTK_IMPORTER`: The VTK importer class to use.
  * `VTK_READER`: The VTK reader class to use.
  * `FORMAT_DESCRIPTION`: The description of the format read by the reader.
  * `SCORE`: The score of the reader (from 0 to 100). Default value is 50.
  * `EXCLUDE_FROM_THUMBNAILER`: If specified, the reader will not be used for generating thumbnails.
  * `CUSTOM_CODE`: A custom code file containing the implementation of ``applyCustomReader`` function.
  * `EXTENSIONS`: (Required) The list of file extensions supported by the reader.
  * `MIMETYPES`: (Required) The list of mimetypes supported by the reader.

#]==]

macro(f3d_plugin_declare_reader)
  cmake_parse_arguments(F3D_READER "EXCLUDE_FROM_THUMBNAILER" "NAME;VTK_IMPORTER;VTK_READER;FORMAT_DESCRIPTION;SCORE;CUSTOM_CODE" "EXTENSIONS;MIMETYPES" ${ARGN})

  if(F3D_READER_CUSTOM_CODE)
    set(F3D_READER_HAS_CUSTOM_CODE 1)
  else()
    set(F3D_READER_HAS_CUSTOM_CODE 0)
  endif()

  if(F3D_READER_SCORE)
    set(F3D_READER_HAS_SCORE 1)
  else()
    set(F3D_READER_HAS_SCORE 0)
  endif()

  set(F3D_PLUGIN_INCLUDES_CODE
    "${F3D_PLUGIN_INCLUDES_CODE}#include \"reader_${F3D_READER_NAME}.h\"\n")
  set(F3D_PLUGIN_REGISTER_CODE
    "${F3D_PLUGIN_REGISTER_CODE}std::make_shared<reader_${F3D_READER_NAME}>(),")

  list(APPEND F3D_PLUGIN_MIMETYPES ${F3D_READER_MIMETYPES})

  set(F3D_READER_JSON "{}")

  string(JSON F3D_READER_JSON
    SET "${F3D_READER_JSON}" "name" "\"${F3D_READER_NAME}\"")

  string(JSON F3D_READER_JSON
    SET "${F3D_READER_JSON}" "description" "\"${F3D_READER_FORMAT_DESCRIPTION}\"")

  list(TRANSFORM F3D_READER_EXTENSIONS PREPEND "\"")
  list(TRANSFORM F3D_READER_EXTENSIONS APPEND "\"")
  list(JOIN F3D_READER_EXTENSIONS ", " F3D_READER_EXTENSIONS)

  string(JSON F3D_READER_JSON
    SET "${F3D_READER_JSON}" "extensions" "[${F3D_READER_EXTENSIONS}]")

  set(F3D_READER_MIMETYPES_THUMB ${F3D_READER_MIMETYPES})
  list(TRANSFORM F3D_READER_MIMETYPES PREPEND "\"")
  list(TRANSFORM F3D_READER_MIMETYPES APPEND "\"")
  list(JOIN F3D_READER_MIMETYPES ", " F3D_READER_MIMETYPES)

  string(JSON F3D_READER_JSON
    SET "${F3D_READER_JSON}" "mimetypes" "[${F3D_READER_MIMETYPES}]")

  if (F3D_READER_EXCLUDE_FROM_THUMBNAILER)
    string(JSON F3D_READER_JSON
      SET "${F3D_READER_JSON}" "exclude_thumbnailer" "true")
  else()
    list(APPEND F3D_PLUGIN_MIMETYPES_THUMB ${F3D_READER_MIMETYPES_THUMB})
    string(JSON F3D_READER_JSON
      SET "${F3D_READER_JSON}" "exclude_thumbnailer" "false")
  endif()

  string(JSON F3D_PLUGIN_JSON
    SET "${F3D_PLUGIN_JSON}" "readers" ${F3D_PLUGIN_CURRENT_READER_INDEX} "${F3D_READER_JSON}")

  math(EXPR "F3D_PLUGIN_CURRENT_READER_INDEX" "${F3D_PLUGIN_CURRENT_READER_INDEX} +1")

  if(F3D_READER_VTK_IMPORTER)
    set(F3D_READER_HAS_SCENE_READER 1)
  else()
    set(F3D_READER_HAS_SCENE_READER 0)
  endif()

  if(F3D_READER_VTK_READER)
    set(F3D_READER_HAS_GEOMETRY_READER 1)
  else()
    set(F3D_READER_HAS_GEOMETRY_READER 0)
  endif()

  configure_file("${_f3dPlugin_dir}/readerBoilerPlate.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/reader_${F3D_READER_NAME}.h")
endmacro()

#[==[
@brief Build the plugin.

~~~
f3d_plugin_build(
  NAME                   <string>
  [DESCRIPTION           <string>]
  [VERSION               <string>]
  [VTK_MODULES           <string>...]
  [ADDITIONAL_RPATHS     <path>...]
  [MIMETYPE_XML_FILES    <path>...]
  [CONFIGURATION_DIRS    <path>...]
  [FREEDESKTOP]
  [FORCE_STATIC])
~~~

Declare the plugin. Must be called after ``f3d_plugin_declare_reader``.

The `NAME` argument is required. The arguments are as follows:

  * `NAME`: (Required) The name of the plugin.
  * `DESCRIPTION`: The description of the plugin.
  * `VERSION`: The version of the plugin.
  * `VTK_MODULES`: The list of VTK modules used by the plugin to link with. A few VTK modules are automatically linked.
  * `ADDITIONAL_RPATHS`: The list of additional RPATH for the installed binaries on Unix. VTK path is added automatically.
  * `MIMETYPE_XML_FILES`: The list of mimetype files to install. It's useful for file association on OS using Freedesktop specifications.
  * `CONFIGURATION_DIRS`: The list of configuration directories to install. Generally contain a load-plugins option and format specific options.
  * `FREEDESKTOP`: If specified, generates .desktop and .thumbnailer used for desktop integration on Linux.
  * `FORCE_STATIC`: If specified, the plugin is built as a static library and embedded into libf3d.
#]==]

macro(f3d_plugin_build)
  cmake_parse_arguments(F3D_PLUGIN "FREEDESKTOP;FORCE_STATIC" "NAME;DESCRIPTION;VERSION" "VTK_MODULES;ADDITIONAL_RPATHS;MIMETYPE_XML_FILES;CONFIGURATION_DIRS" ${ARGN})

  find_package(VTK 9.0 REQUIRED COMPONENTS
    CommonCore
    CommonExecutionModel
    IOImport
    ${F3D_PLUGIN_VTK_MODULES})

  if(F3D_PLUGIN_FORCE_STATIC OR F3D_PLUGINS_STATIC_BUILD)
    set(F3D_PLUGIN_TYPE "STATIC")
    set(F3D_PLUGIN_IS_STATIC ON)
    set_property(GLOBAL APPEND PROPERTY F3D_STATIC_PLUGINS ${F3D_PLUGIN_NAME})
  else()
    set(F3D_PLUGIN_TYPE "MODULE")
    set(F3D_PLUGIN_IS_STATIC OFF)

    get_target_property(target_type VTK::CommonCore TYPE)
    if (target_type STREQUAL STATIC_LIBRARY)
      message(WARNING "\"${F3D_PLUGIN_NAME}\" plugin is built as a shared module, but VTK is built as a static library. "
                      "It will result in a large plugin binary and unexpected behavior. "
                      "If the plugin is internal, consider building it with FORCE_STATIC, "
                      "enable option F3D_PLUGINS_STATIC_BUILD, or rebuild VTK as a shared library.")
    endif ()
  endif()

  set(f3d_plugin_compile_options "")
  list(APPEND f3d_plugin_compile_options "${f3d_strict_build_compile_options}")
  list(APPEND f3d_plugin_compile_options "${f3d_coverage_compile_options}")
  list(APPEND f3d_plugin_compile_options "${f3d_sanitizer_compile_options}")

  set(f3d_plugin_link_options "")
  list(APPEND f3d_plugin_link_options "${f3d_coverage_link_options}")
  list(APPEND f3d_plugin_link_options "${f3d_sanitizer_link_options}")

  # if libf3d is built as a static library, VTK extension static libraries
  # must be exported as well for proper linkage
  set(export_name "")
  if(NOT BUILD_SHARED_LIBS AND F3D_PLUGIN_IS_STATIC)
    set(export_name "f3dTargets")
  endif()

  vtk_module_find_modules(vtk_module_files "${CMAKE_CURRENT_SOURCE_DIR}")

  if(NOT ${vtk_module_files} STREQUAL "")
    vtk_module_scan(
      MODULE_FILES            ${vtk_module_files}
      PROVIDES_MODULES        modules
      WANT_BY_DEFAULT         ON
      HIDE_MODULES_FROM_CACHE ON
      ENABLE_TESTS            ${BUILD_TESTING})

    vtk_module_build(
      MODULES ${modules}
      INSTALL_EXPORT ${export_name}
      INSTALL_HEADERS OFF
      HEADERS_COMPONENT vtkext
      TARGETS_COMPONENT vtkext
      PACKAGE "f3d-plugin-${F3D_PLUGIN_NAME}")

    foreach (module IN LISTS modules)
      if(NOT "${f3d_plugin_compile_options}" STREQUAL "")
        vtk_module_compile_options(${module} PUBLIC "${f3d_plugin_compile_options}")
      endif()
      if(NOT "${f3d_plugin_link_options}" STREQUAL "")
        vtk_module_link_options(${module} PUBLIC "${f3d_plugin_link_options}")
      endif()
    endforeach ()
  else()
    set(modules "")
  endif()

  configure_file("${_f3dPlugin_dir}/plugin.cxx.in" "${CMAKE_CURRENT_BINARY_DIR}/plugin.cxx")

  add_library(f3d-plugin-${F3D_PLUGIN_NAME} ${F3D_PLUGIN_TYPE} "${CMAKE_CURRENT_BINARY_DIR}/plugin.cxx")

  include(GenerateExportHeader)
  generate_export_header(f3d-plugin-${F3D_PLUGIN_NAME}
    EXPORT_FILE_NAME plugin_export.h
    EXPORT_MACRO_NAME F3D_PLUGIN_EXPORT)

  if(UNIX)
    # On Unix, add the RPATH to VTK install folder
    get_target_property(target_type VTK::CommonCore TYPE)
    if (target_type STREQUAL SHARED_LIBRARY)
      list(APPEND F3D_PLUGIN_ADDITIONAL_RPATHS "$<TARGET_FILE_DIR:VTK::CommonCore>")
    endif ()
    set_target_properties(f3d-plugin-${F3D_PLUGIN_NAME} PROPERTIES
      INSTALL_RPATH "${F3D_PLUGIN_ADDITIONAL_RPATHS}")
  endif()

  set_target_properties(f3d-plugin-${F3D_PLUGIN_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    )

  if(DEFINED f3d_INCLUDE_DIR)
    # External plugin path
    set(_f3d_include_path "${f3d_INCLUDE_DIR}/f3d")
    set(_f3d_config_dir ${f3d_CONFIG_DIR})
    set(_f3d_plugins_install_dir ${f3d_PLUGINS_INSTALL_DIR})
  else()
    # In-source plugin path
    set(_f3d_include_path "${CMAKE_SOURCE_DIR}/library/plugin")
    set(_f3d_config_dir ${f3d_config_dir})
    set(_f3d_plugins_install_dir ${F3D_PLUGINS_INSTALL_DIR})
  endif()

  target_include_directories(f3d-plugin-${F3D_PLUGIN_NAME}
    PRIVATE
      "${_f3d_include_path}"
      "${CMAKE_CURRENT_BINARY_DIR}")

  target_compile_options(f3d-plugin-${F3D_PLUGIN_NAME} PUBLIC "${f3d_plugin_compile_options}")
  target_link_options(f3d-plugin-${F3D_PLUGIN_NAME} PUBLIC "${f3d_plugin_link_options}")

  list(TRANSFORM F3D_PLUGIN_VTK_MODULES PREPEND "VTK::")

  target_link_libraries(f3d-plugin-${F3D_PLUGIN_NAME} PRIVATE
    VTK::CommonCore VTK::CommonExecutionModel VTK::IOImport
    ${F3D_PLUGIN_VTK_MODULES}
    ${modules})

  if(NOT F3D_PLUGIN_IS_STATIC OR NOT BUILD_SHARED_LIBS)
    install(TARGETS f3d-plugin-${F3D_PLUGIN_NAME}
      EXPORT ${export_name}
      ARCHIVE DESTINATION ${_f3d_plugins_install_dir} COMPONENT plugin
      LIBRARY DESTINATION ${_f3d_plugins_install_dir} COMPONENT plugin)
  endif()

  # Install configurations folders
  foreach(config_dir ${F3D_PLUGIN_CONFIGURATION_DIRS})
    install(
      DIRECTORY "${config_dir}"
      DESTINATION "${_f3d_config_dir}"
      COMPONENT configuration
      EXCLUDE_FROM_ALL)
  endforeach()

  # Generate mime, desktop and thumbnailer files
  if (UNIX AND NOT APPLE AND NOT ANDROID)
    foreach(mimetype_xml ${F3D_PLUGIN_MIMETYPE_XML_FILES})
      install(FILES "${mimetype_xml}"
        DESTINATION "share/mime/packages"
        COMPONENT mimetypes
        EXCLUDE_FROM_ALL)
    endforeach()

    if(F3D_PLUGIN_FREEDESKTOP)
      configure_file(
        "${_f3dPlugin_dir}/plugin.desktop.in"
        "${CMAKE_BINARY_DIR}/share/applications/f3d-plugin-${F3D_PLUGIN_NAME}.desktop")
      configure_file(
        "${_f3dPlugin_dir}/plugin.thumbnailer.in"
        "${CMAKE_BINARY_DIR}/share/thumbnailers/f3d-plugin-${F3D_PLUGIN_NAME}.thumbnailer")
      install(FILES "${CMAKE_BINARY_DIR}/share/applications/f3d-plugin-${F3D_PLUGIN_NAME}.desktop"
        DESTINATION "share/applications"
        COMPONENT plugin)
      install(FILES "${CMAKE_BINARY_DIR}/share/thumbnailers/f3d-plugin-${F3D_PLUGIN_NAME}.thumbnailer"
        DESTINATION "share/thumbnailers"
        COMPONENT plugin)
    endif()
  endif()

  #[==[
  Generate a JSON file describing the plugin
  Example:
  {
    "description" : "Plugin description",
    "name" : "myPlugin",
    "readers" :
    [
      {
        "description" : "Reader description",
        "extensions" : [ "myext" ],
        "mimetypes" : [ "application/vnd.myext" ],
        "name" : "myReader"
      }
    ],
    "type" : "MODULE",
    "version" : "1.0"
  }
  #]==]

  string(JSON F3D_PLUGIN_JSON
    SET "${F3D_PLUGIN_JSON}" "name" "\"${F3D_PLUGIN_NAME}\"")

  string(JSON F3D_PLUGIN_JSON
    SET "${F3D_PLUGIN_JSON}" "description" "\"${F3D_PLUGIN_DESCRIPTION}\"")

  string(JSON F3D_PLUGIN_JSON
    SET "${F3D_PLUGIN_JSON}" "version" "\"${F3D_PLUGIN_VERSION}\"")

  string(JSON F3D_PLUGIN_JSON
    SET "${F3D_PLUGIN_JSON}" "type" "\"${F3D_PLUGIN_TYPE}\"")

  set(F3D_PLUGIN_JSON_FILE "${CMAKE_BINARY_DIR}/share/f3d/plugins/${F3D_PLUGIN_NAME}.json")

  file(WRITE "${F3D_PLUGIN_JSON_FILE}" "${F3D_PLUGIN_JSON}")

  install(FILES "${F3D_PLUGIN_JSON_FILE}"
      DESTINATION "share/f3d/plugins"
      COMPONENT plugin)
endmacro()
