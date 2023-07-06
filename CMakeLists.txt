cmake_minimum_required(VERSION 3.21)
include(CMakeDependentOption)

# Generic CMake variables, should be done before project()
set(output_postfix "")
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(${is_multi_config})
  set(output_postfix "_$<CONFIG>")
  if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Multi-config types" FORCE)
  endif()
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
  endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin${output_postfix}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib${output_postfix}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib${output_postfix}")

project(F3D
  DESCRIPTION "F3D - A fast and minimalist 3D viewer"
  LANGUAGES C CXX)

set(f3d_cmake_dir "${F3D_SOURCE_DIR}/cmake")
 list(INSERT CMAKE_MODULE_PATH 0 "${f3d_cmake_dir}")
include(f3dVersion)

# Handle F3D_VERSION
set(F3D_MAJOR_VERSION 2)
set(F3D_MINOR_VERSION 1)
set(F3D_PATCH_VERSION 0)
set(F3D_VERSION ${F3D_MAJOR_VERSION}.${F3D_MINOR_VERSION}.${F3D_PATCH_VERSION})
set(F3D_VERSION_FULL ${F3D_VERSION})
find_package(Git QUIET)
f3d_determine_version("${CMAKE_CURRENT_SOURCE_DIR}" "${GIT_EXECUTABLE}" "F3D")

string(TIMESTAMP F3D_BUILD_DATE "%Y-%m-%d %H:%M:%S" UTC)

math(EXPR F3D_SYSTEM_PROCESSOR "8 * ${CMAKE_SIZEOF_VOID_P}")
set(F3D_SYSTEM_PROCESSOR "${F3D_SYSTEM_PROCESSOR}-bits")

# Modules
option(F3D_MODULE_EXTERNAL_RENDERING "External rendering module" OFF)
option(F3D_MODULE_RAYTRACING "Raytracing module" OFF)
option(F3D_MODULE_EXR "OpenEXR images module" OFF)

# Use externals
option(F3D_USE_EXTERNAL_CXXOPTS "Use external cxxopts dependency" OFF)
option(F3D_USE_EXTERNAL_NLOHMANN_JSON "Use external nlohmann_json dependency" OFF)

if (F3D_USE_EXTERNAL_CXXOPTS)
  find_package(cxxopts REQUIRED)
endif ()
if (F3D_USE_EXTERNAL_NLOHMANN_JSON)
  find_package(nlohmann_json REQUIRED)
endif ()

# VTK dependency
find_package(VTK 9.0 REQUIRED
  COMPONENTS
    CommonCore
    CommonDataModel
    CommonExecutionModel
    FiltersGeneral
    FiltersGeometry
    ImagingCore
    ImagingHybrid
    InteractionStyle
    InteractionWidgets
    IOCityGML
    IOGeometry
    IOImage
    IOImport
    IOParallel
    IOPLY
    IOXML
    RenderingAnnotation
    RenderingCore
    RenderingLabel
    RenderingOpenGL2
    RenderingVolumeOpenGL2
    TestingCore
    jsoncpp
    opengl
  OPTIONAL_COMPONENTS
    IOExodus
    RenderingExternal
    RenderingRayTracing)
message(STATUS "VTK ${VTK_VERSION} found")

# Shared options between application and library
include(GNUInstallDirs)
cmake_dependent_option(F3D_WINDOWS_GUI "Build a non-console Win32 application" ON "WIN32" OFF)
cmake_dependent_option(F3D_MACOS_BUNDLE "Build a macOS bundle application" ON "APPLE" OFF)

# Force static library when creating a macOS bundle
cmake_dependent_option(BUILD_SHARED_LIBS "Build f3d with shared libraries" ON "NOT ANDROID" OFF)

# F3D_STRICT_BUILD
set(F3D_STRICT_BUILD OFF CACHE BOOL "Use strict warnings and errors flags for building F3D")
mark_as_advanced(F3D_STRICT_BUILD)
set(f3d_strict_build_compile_options "")
if(F3D_STRICT_BUILD)
  if(MSVC)
    set(f3d_strict_build_compile_options /W4 /WX)
  else()
    set(f3d_strict_build_compile_options -Wall -Wextra -Wshadow -Woverloaded-virtual -Wno-deprecated -Wno-strict-overflow -Wno-array-bounds -Wunreachable-code -Wno-missing-field-initializers -Wno-unused-parameter -Wredundant-decls -Wpointer-arith -Werror)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      list(APPEND f3d_strict_build_compile_options -Wsuggest-override)
    endif()
  endif()
endif()

# Coverage
cmake_dependent_option(F3D_COVERAGE "Emit coverage files" OFF "UNIX" OFF)
set(f3d_coverage_compile_options "")
set(f3d_coverage_link_options "")
if(F3D_COVERAGE)
  set(f3d_coverage_compile_options -g -O0 --coverage)
  set(f3d_coverage_link_options --coverage)
endif()

# Sanitizer
if(NOT F3D_SANITIZER)
  set(F3D_SANITIZER "none" CACHE STRING "Sanitizer type" FORCE)
  set_property(CACHE F3D_SANITIZER PROPERTY STRINGS "none" "address" "thread" "leak" "memory" "undefined")
endif()

if(NOT UNIX)
  set_property(CACHE F3D_SANITIZER PROPERTY TYPE INTERNAL)
endif()

set(f3d_sanitizer_compile_options "")
set(f3d_sanitizer_link_options "")
if(NOT F3D_SANITIZER STREQUAL "none")
  set(f3d_sanitizer_compile_options -fsanitize=${F3D_SANITIZER} -fno-optimize-sibling-calls -fno-omit-frame-pointer -g)
  if(${F3D_SANITIZER} STREQUAL "address")
    list(APPEND f3d_sanitizer_compile_options -fsanitize-address-use-after-scope)
  endif()
  if(${F3D_SANITIZER} STREQUAL "memory")
    list(APPEND f3d_sanitizer_compile_options -fsanitize-memory-track-origins)
  endif()
  set(f3d_sanitizer_link_options -fsanitize=${F3D_SANITIZER})
endif()

# Testing
option(BUILD_TESTING "Build the tests" OFF)
cmake_dependent_option(F3D_TESTING_DISABLE_DEFAULT_LIGHTS_TESTS_COMPARISON "Disable image comparison on tests using the default lights" OFF "BUILD_TESTING" ON)
cmake_dependent_option(F3D_TESTING_ENABLE_RENDERING_TESTS "Enable rendering tests" ON "BUILD_TESTING" ON)
cmake_dependent_option(F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS "Enable long timeout tests" OFF "BUILD_TESTING" ON)
cmake_dependent_option(F3D_TESTING_ENABLE_HDRI_TESTS "Enable HDRi related tests" ON "F3D_TESTING_RENDRERING_TESTS" ON "F3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS" ON "BUILD_TESTING" ON)
if(BUILD_TESTING)
  enable_testing()
endif()

# Figure out F3D configuration directory
if(UNIX AND NOT APPLE)
  option(F3D_LINUX_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX "Install the default configuration at the prefix root instead of system wide" OFF)
  mark_as_advanced(F3D_LINUX_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX)
endif()

set(f3d_resources_dir "share/f3d")
if (F3D_MACOS_BUNDLE)
  set(f3d_resources_dir "f3d.app/Contents/Resources")
endif()

## Will be stored in f3dConfig.cmake
set(f3d_config_dir "${f3d_resources_dir}/configs")
if (UNIX AND NOT APPLE)
  if (NOT F3D_LINUX_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX)
    set(f3d_config_dir "${CMAKE_INSTALL_FULL_SYSCONFDIR}/f3d")
  endif()
endif()

# plugins
option(F3D_PLUGINS_STATIC_BUILD "Make all plugins static (embedded into libf3d) and automatically loaded by F3D" ON)
mark_as_advanced(F3D_PLUGINS_STATIC_BUILD)
if (F3D_MACOS_BUNDLE AND NOT F3D_PLUGINS_STATIC_BUILD)
  message(FATAL_ERROR "Building a macOS Bundle do not support loading plugins dynamically")
endif()
set(F3D_PLUGINS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}" CACHE STRING "Plugins install directory relative path")
mark_as_advanced(F3D_PLUGINS_INSTALL_DIR)
add_subdirectory(plugins)

# libf3d target
add_subdirectory(library)

# F3D Application
cmake_dependent_option(F3D_BUILD_APPLICATION "Build the application" ON "NOT ANDROID AND NOT EMSCRIPTEN" OFF)
if (F3D_BUILD_APPLICATION)
  add_subdirectory(application)
endif()

# Windows Shell Extension
cmake_dependent_option(F3D_WINDOWS_BUILD_SHELL_THUMBNAILS_EXTENSION "Build the Windows Shell Extension to produce thumbnails" ON "WIN32" OFF)
if(F3D_WINDOWS_BUILD_SHELL_THUMBNAILS_EXTENSION)
  add_subdirectory(winshellext)
endif()

# Python bindings
option(F3D_BINDINGS_PYTHON "Create Python bindings" OFF)
if(F3D_BINDINGS_PYTHON)
  add_subdirectory(python)
endif()

# JNI bindings
option(F3D_BINDINGS_JAVA "Create Java bindings" OFF)
if(F3D_BINDINGS_JAVA)
  add_subdirectory(java)
endif()

# webassembly bindings
if(EMSCRIPTEN)
  add_subdirectory(webassembly)
endif ()

# Installing licenses
set(F3D_LIC_DIR ".")

if (UNIX AND NOT APPLE AND NOT ANDROID)
  set(F3D_LIC_DIR ${CMAKE_INSTALL_DOCDIR})
endif()

install(FILES LICENSE.md
  DESTINATION ${F3D_LIC_DIR} COMPONENT licenses)

if (F3D_BUILD_APPLICATION)
  install(FILES doc/THIRD_PARTY_LICENSES.md
    DESTINATION ${F3D_LIC_DIR} COMPONENT licenses)
endif ()

# Installling resources
add_subdirectory(resources)
