# Define VTK modules to link libf3d with
# Note that readers/importers specific modules
# should be listed here as reader instantiation is header only
set(F3D_VTK_MODULES
  VTK::CommonSystem
  VTK::IOImage
  VTK::InteractionWidgets
  f3d::vtkext
  f3d::vtkextPrivate
)

# Check modules
if(F3D_MODULE_RAYTRACING)
  if(TARGET VTK::RenderingRayTracing)
    message(STATUS "VTK::RenderingRayTracing module found")
    set(F3D_VTK_MODULES ${F3D_VTK_MODULES} VTK::RenderingRayTracing)
  else()
    message(FATAL_ERROR "F3D_MODULE_RAYTRACING option is enabled but VTK::RenderingRayTracing is not found")
  endif()
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/src/config.cxx"
  @ONLY)

include(f3dOptions)
f3d_generate_options(
  INPUT_JSON "${CMAKE_CURRENT_SOURCE_DIR}/options.json"
  INPUT_PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/public/options.h.in"
  INPUT_PRIVATE_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/private/options_tools.h.in"
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
  OUTPUT_NAME "options"
  )

# Define libf3d target
set(F3D_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/animationManager.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/camera_impl.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/src/config.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/context.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/engine.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/src/factory.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/image.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/init.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/interactor.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/interactor_impl.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/levenshtein.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/log.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/options.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/scene_impl.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/types.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/src/window_impl.cxx
  )

if(APPLE)
  list(APPEND F3D_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/context_cocoa.mm)
endif()

# List of headers that will be installed
set(F3D_PUBLIC_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/public/export.h
  ${CMAKE_CURRENT_BINARY_DIR}/public/options.h
  ${CMAKE_CURRENT_SOURCE_DIR}/plugin/plugin.h
  ${CMAKE_CURRENT_SOURCE_DIR}/plugin/reader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/camera.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/context.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/engine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/exception.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/interactor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/log.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/scene.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/public/window.h
  )

set(F3D_PLUGIN_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/plugin/reader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/plugin/plugin.h
  )

add_library(libf3d
  ${F3D_SOURCE_FILES}
  )

target_include_directories(libf3d
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/public>
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/private>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/plugin>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/plugin>
  )
if (F3D_USE_EXTERNAL_NLOHMANN_JSON)
  target_link_libraries(libf3d PRIVATE nlohmann_json::nlohmann_json)
else ()
  target_include_directories(libf3d PRIVATE
    $<BUILD_INTERFACE:${F3D_SOURCE_DIR}/external/nlohmann_json>)
endif ()

if (WIN32)
  target_link_libraries(libf3d PRIVATE Dwmapi)
endif()

set_target_properties(libf3d PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  CXX_STANDARD 17
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME "f3d"
  PDB_NAME "libf3d"
  )

# It can be useful to disable soversion in case the links are duplicated
# It happens with Python wheels for example
option(F3D_ENABLE_SOVERSION "Enable libf3d SOVERSION" ON)
mark_as_advanced(F3D_ENABLE_SOVERSION)
if(F3D_ENABLE_SOVERSION)
  set_target_properties(libf3d PROPERTIES
    VERSION ${F3D_MAJOR_VERSION}.${F3D_MINOR_VERSION}
    SOVERSION ${F3D_MAJOR_VERSION}
  )
endif()

# Generate export macros for exported public APIs
include(GenerateExportHeader)
set(libf3d_no_deprecated)
if(F3D_EXCLUDE_DEPRECATED)
  set(libf3d_no_deprecated "DEFINE_NO_DEPRECATED")
endif()
generate_export_header(libf3d
  EXPORT_FILE_NAME public/export.h
  BASE_NAME F3D
  ${libf3d_no_deprecated})

target_include_directories(libf3d
  INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

vtk_module_autoinit(TARGETS libf3d MODULES ${F3D_VTK_MODULES})

# The initialize function pointer of the static plugins must be added to the map of static plugins
# in the factory constructor. When loading a plugin, there's a lookup into this map.
get_property(F3D_STATIC_PLUGIN_TARGETS GLOBAL PROPERTY F3D_STATIC_PLUGINS)
set(F3D_STATIC_PLUGIN_MAP "${F3D_STATIC_PLUGIN_TARGETS}")
set(F3D_STATIC_PLUGIN_EXTERN "${F3D_STATIC_PLUGIN_TARGETS}")

list(TRANSFORM F3D_STATIC_PLUGIN_EXTERN REPLACE "^(.+)$" "extern f3d::plugin* init_plugin_static_\\1()\\\;")
list(JOIN F3D_STATIC_PLUGIN_EXTERN "\n" F3D_STATIC_PLUGIN_EXTERN)

list(TRANSFORM F3D_STATIC_PLUGIN_MAP REPLACE "^(.+)$" "this->StaticPluginInitializers[\"\\1\"] = init_plugin_static_\\1\\\;")
list(JOIN F3D_STATIC_PLUGIN_MAP "\n  " F3D_STATIC_PLUGIN_MAP)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/factory.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/src/factory.cxx")

# libf3d links with the static plugins.
list(TRANSFORM F3D_STATIC_PLUGIN_TARGETS PREPEND "f3d-plugin-")

target_link_libraries(libf3d PRIVATE ${F3D_VTK_MODULES} ${F3D_STATIC_PLUGIN_TARGETS})
target_compile_options(libf3d PUBLIC ${f3d_compile_options_public} PRIVATE ${f3d_compile_options_private})
target_link_options(libf3d PUBLIC ${f3d_link_options_public})

if (F3D_MODULE_UI)
  target_compile_definitions(libf3d PRIVATE F3D_MODULE_UI)
endif ()

if(F3D_STRICT_BUILD AND MSVC)
  # There are warnings in VTK related to deprecated features in C++17
  target_compile_definitions(libf3d PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif()

if(APPLE)
  set_target_properties(libf3d PROPERTIES INSTALL_RPATH "@loader_path")
elseif(UNIX)
  set_target_properties(libf3d PROPERTIES INSTALL_RPATH "$ORIGIN")
endif()

if (APPLE OR UNIX)
  # Make sure rpath is correctly set for libf3d
  get_target_property(target_type VTK::CommonCore TYPE)
  if (target_type STREQUAL SHARED_LIBRARY)
    set_target_properties(libf3d PROPERTIES BUILD_RPATH "$<TARGET_FILE_DIR:VTK::CommonCore>")
  endif ()
endif ()

if(UNIX AND NOT APPLE)
  option(F3D_LINUX_LIBRARY_LINK_ATOMIC "Link with libatomic" OFF)
  mark_as_advanced(F3D_LINUX_LIBRARY_LINK_ATOMIC)
  if(F3D_LINUX_LIBRARY_LINK_ATOMIC)
    target_link_libraries(libf3d PRIVATE atomic)
  endif()
endif()

# Testing
if(BUILD_TESTING)
  add_subdirectory(testing)
endif()

# Installing

## Install f3dConfig.cmake and f3dVersion.cmake so the f3d::f3d target can be found
## Even without BUILD_SHARED_LIBS
include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${F3D_SOURCE_DIR}/cmake/f3dConfig.cmake.in" "${CMAKE_BINARY_DIR}/cmake/f3dConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/f3d")
write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/cmake/f3dConfigVersion.cmake"
  VERSION "${F3D_VERSION}"
  COMPATIBILITY SameMajorVersion)
install(
  FILES
    "${CMAKE_BINARY_DIR}/cmake/f3dConfig.cmake"
    "${CMAKE_BINARY_DIR}/cmake/f3dConfigVersion.cmake"
  DESTINATION
    "${CMAKE_INSTALL_LIBDIR}/cmake/f3d"
  COMPONENT sdk
  EXCLUDE_FROM_ALL)

## Install the rest of the libraries and SDK parts
if(BUILD_SHARED_LIBS)

  # Install the libf3d
  install(TARGETS libf3d
    EXPORT f3dLibraryTargets
    RUNTIME_DEPENDENCY_SET libf3dDeps
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT sdk EXCLUDE_FROM_ALL
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT library
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT library)

  # Install dependencies, but exclude system libraries
  set(F3D_DEPENDENCIES_DIR "" CACHE STRING "Semicolon separated directories for dependencies look up")
  mark_as_advanced(F3D_DEPENDENCIES_DIR)
  install(RUNTIME_DEPENDENCY_SET libf3dDeps
    COMPONENT dependencies EXCLUDE_FROM_ALL
    PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
    POST_EXCLUDE_REGEXES ".*system32/.*" "^/usr/lib.*" "^/lib.*" "^/var/lib.*"
    DIRECTORIES "${F3D_DEPENDENCIES_DIR}")

  # Install the public headers
  install(FILES ${F3D_PUBLIC_HEADERS}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/f3d"
    COMPONENT sdk
    EXCLUDE_FROM_ALL)

  # Install the library exported targets
  install(EXPORT f3dLibraryTargets
    NAMESPACE f3d::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/f3d"
    COMPONENT sdk
    EXCLUDE_FROM_ALL)

  # Install library cmake files
  install(
    FILES
      "${F3D_SOURCE_DIR}/cmake/library-config.cmake"
      "${F3D_SOURCE_DIR}/cmake/f3dEmbed.cmake"
    DESTINATION
      "${CMAKE_INSTALL_LIBDIR}/cmake/f3d"
    COMPONENT sdk
    EXCLUDE_FROM_ALL)

  # Install plugin headers
  install(FILES ${F3D_PLUGIN_HEADERS}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/f3d"
    COMPONENT plugin_sdk
    EXCLUDE_FROM_ALL)

  # Install pluginsdk cmake and source files
  install(
    FILES
      "${F3D_SOURCE_DIR}/cmake/pluginsdk-config.cmake"
      "${F3D_SOURCE_DIR}/cmake/f3dPlugin.cmake"
      "${F3D_SOURCE_DIR}/cmake/plugin.cxx.in"
      "${F3D_SOURCE_DIR}/cmake/plugin.desktop.in"
      "${F3D_SOURCE_DIR}/cmake/plugin.thumbnailer.in"
      "${F3D_SOURCE_DIR}/cmake/readerBoilerPlate.h.in"
    DESTINATION
      "${CMAKE_INSTALL_LIBDIR}/cmake/f3d"
    COMPONENT plugin_sdk
    EXCLUDE_FROM_ALL)

endif()
