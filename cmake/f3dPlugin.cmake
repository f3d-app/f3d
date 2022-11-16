#[==[
@brief Initialize the plugin.

~~~
f3d_plugin_init()
~~~

This initializes the variable used internally to accumulate the reader declarations.
#]==]

macro(f3d_plugin_init)
  set(F3D_READER_INCLUDES_CODE "")
  set(F3D_READER_INSTANCES_CODE "")
  set(F3D_READER_REGISTER_CODE "")
  set(F3D_READER_UNREGISTER_CODE "")
endmacro()

#[==[
@brief Declare a reader.

~~~
f3d_plugin_declare_reader(
  NAME                   <string>
  [VTK_IMPORTER          <class>]
  [VTK_READER            <class>]
  [DESCRIPTION           <string>]
  [SCORE                 <integer>]
  [CUSTOM_CODE           <file>]
  EXTENSIONS             <string>...
  MIMETYPES              <string>...)
~~~

Declare a reader. Can be called several times and must be called after ``f3d_plugin_init``.

The `NAME` argument is required. The arguments are as follows:

  * `NAME`: (Required) The name of the reader.
  * `VTK_IMPORTER`: The VTK importer class to use.
  * `VTK_READER`: The VTK reader class to use.
  * `DESCRIPTION`: The description of the reader.
  * `SCORE`: The score of the reader (from 0 to 100). Default value is 50.
  * `CUSTOM_CODE`: A custom code file containing the implementation of ``applyCustomReader`` function.
  * `EXTENSIONS`: (Required) The list of file extensions supported by the reader.
  * `MIMETYPES`: (Required) The list of mimetypes supported by the reader.

#]==]

macro(f3d_plugin_declare_reader)
  cmake_parse_arguments(F3D_READER "" "NAME;VTK_IMPORTER;VTK_READER;DESCRIPTION;SCORE;CUSTOM_CODE" "EXTENSIONS;MIMETYPES" ${ARGN})

  if(F3D_READER_CUSTOM_CODE)
    set(F3D_READER_HAS_CUSTOM_CODE 1)
  else()
    set(F3D_READER_HAS_CUSTOM_CODE 0)
  endif()

  if(NOT F3D_READER_SCORE)
    set(F3D_READER_SCORE 50)
  endif()

  set(F3D_READER_INCLUDES_CODE
    "${F3D_READER_INCLUDES_CODE}#include \"reader_${F3D_READER_NAME}.h\"\n")
  set(F3D_READER_REGISTER_CODE
    "${F3D_READER_REGISTER_CODE}std::make_shared<reader_${F3D_READER_NAME}>(),")

  set_property(GLOBAL APPEND PROPERTY F3D_SUPPORTED_EXTENSIONS ${F3D_READER_EXTENSIONS})
  set_property(GLOBAL APPEND PROPERTY F3D_SUPPORTED_MIME_TYPES ${F3D_READER_MIMETYPES})

  list(TRANSFORM F3D_READER_EXTENSIONS PREPEND "\"")
  list(TRANSFORM F3D_READER_EXTENSIONS APPEND "\"")
  list(JOIN F3D_READER_EXTENSIONS ", " F3D_READER_EXTENSIONS)

  list(TRANSFORM F3D_READER_MIMETYPES PREPEND "\"")
  list(TRANSFORM F3D_READER_MIMETYPES APPEND "\"")
  list(JOIN F3D_READER_MIMETYPES ", " F3D_READER_MIMETYPES)

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

  configure_file("${CMAKE_SOURCE_DIR}/cmake/readerBoilerPlate.h.in"
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
  [FORCE_STATIC])
~~~

Declare the plugin. Must be called after ``f3d_plugin_declare_reader``.

The `NAME` argument is required. The arguments are as follows:

  * `NAME`: (Required) The name of the plugin.
  * `DESCRIPTION`: The description of the plugin.
  * `VERSION`: The version of the plugin.
  * `VTK_MODULES`: The list of VTK modules used by the plugin to link with.
  * `ADDITIONAL_RPATHS`: The list of additional RPATH for the installed binaries on Unix. VTK path is added automatically.
  * `FORCE_STATIC`: If specified, the plugin is built as a static library and embedded into libf3d.
#]==]

macro(f3d_plugin_build)
  cmake_parse_arguments(F3D_PLUGIN "FORCE_STATIC" "NAME;DESCRIPTION;VERSION" "VTK_MODULES;ADDITIONAL_RPATHS" ${ARGN})

  if(F3D_PLUGIN_FORCE_STATIC OR F3D_PLUGINS_STATIC_BUILD)
    set(F3D_PLUGIN_TYPE "STATIC")
    set(F3D_PLUGIN_IS_STATIC ON)
    set_property(GLOBAL APPEND PROPERTY F3D_STATIC_PLUGINS ${F3D_PLUGIN_NAME})
  else()
    set(F3D_PLUGIN_TYPE "MODULE")
    set(F3D_PLUGIN_IS_STATIC OFF)

    get_target_property(target_type VTK::CommonCore TYPE)
    if (target_type STREQUAL STATIC_LIBRARY)
      message(FATAL_ERROR "\"${F3D_PLUGIN_NAME}\" plugin is built as a shared module, but VTK is built as a static library. Please flag the plugin as FORCE_STATIC, enable option F3D_PLUGINS_STATIC_BUILD, or rebuild VTK as a shared library.")
    endif ()
  endif()

  # if libf3d is built as a static library, VTK extension static libraries
  # must be exported as well for proper linkage
  set(export_name "")
  if(NOT BUILD_SHARED_LIBS AND F3D_PLUGIN_IS_STATIC)
    set(export_name "f3dTargets")
  endif()

  find_package(VTK 9.0 REQUIRED COMPONENTS ${F3D_PLUGIN_VTK_MODULES})

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
  else()
    set(modules "")
  endif()

  configure_file("${CMAKE_SOURCE_DIR}/cmake/plugin.cxx.in" "${CMAKE_CURRENT_BINARY_DIR}/plugin.cxx")

  add_library(f3d-plugin-${F3D_PLUGIN_NAME} ${F3D_PLUGIN_TYPE} "${CMAKE_CURRENT_BINARY_DIR}/plugin.cxx")

  include(GenerateExportHeader)
  generate_export_header(f3d-plugin-${F3D_PLUGIN_NAME}
    EXPORT_FILE_NAME plugin_export.h
    EXPORT_MACRO_NAME F3D_PLUGIN_EXPORT)

  if(NOT WIN32)
    get_target_property(target_type VTK::CommonCore TYPE)
    if (target_type STREQUAL SHARED_LIBRARY)
      set_target_properties(f3d-plugin-${F3D_PLUGIN_NAME} PROPERTIES
        INSTALL_RPATH "$<TARGET_FILE_DIR:VTK::CommonCore>;${F3D_PLUGIN_ADDITIONAL_RPATHS}")
    endif ()
  endif()

  if(WIN32)
    set(F3D_PLUGIN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  else()
    set(F3D_PLUGIN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
  endif()

  set_target_properties(f3d-plugin-${F3D_PLUGIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${F3D_PLUGIN_OUTPUT_DIRECTORY}"
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    )

  target_include_directories(f3d-plugin-${F3D_PLUGIN_NAME}
    PRIVATE
      "${CMAKE_SOURCE_DIR}/library/private"
      "${CMAKE_CURRENT_BINARY_DIR}")

  list(TRANSFORM F3D_PLUGIN_VTK_MODULES PREPEND "VTK::")

  target_link_libraries(f3d-plugin-${F3D_PLUGIN_NAME} PUBLIC
    ${F3D_PLUGIN_VTK_MODULES}
    ${modules})

  if (NOT F3D_PLUGIN_IS_STATIC OR NOT BUILD_SHARED_LIBS)
    install(TARGETS f3d-plugin-${F3D_PLUGIN_NAME}
      EXPORT ${export_name}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT plugin
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT plugin
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT plugin)
  endif()
endmacro()
