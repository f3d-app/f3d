cmake_minimum_required(VERSION 3.12)
include(CMakeDependentOption)

project(f3d
  VERSION "1.3.1"
  DESCRIPTION "F3D - A fast and minimalist 3D viewer"
  LANGUAGES C CXX)

string(TIMESTAMP F3D_BUILD_DATE "%Y-%m-%d %H:%M:%S" UTC)

math(EXPR F3D_SYSTEM_PROCESSOR "8 * ${CMAKE_SIZEOF_VOID_P}")
set(F3D_SYSTEM_PROCESSOR "${F3D_SYSTEM_PROCESSOR}-bits")

# Generic CMake variables
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# Modules
option(F3D_MODULE_EXTERNAL_RENDERING "External rendering module" OFF)
option(F3D_MODULE_RAYTRACING "Raytracing module" OFF)

option(F3D_INSTALL_SDK "Install F3D library SDK" OFF)

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
cmake_dependent_option(BUILD_SHARED_LIBS "Build f3d with shared libraries" ON "NOT F3D_MACOS_BUNDLE AND NOT ANDROID" OFF)

# F3D_STRICT_BUILD
set(F3D_STRICT_BUILD OFF CACHE BOOL "Use strict warnings and errors flags for building F3D")
mark_as_advanced(F3D_STRICT_BUILD)
set(f3d_strict_build_compile_options "")
if(F3D_STRICT_BUILD)
  if(MSVC)
    set(f3d_strict_build_compile_options /W4 /WX)
  else()
    set(f3d_strict_build_compile_options -Wall -Wextra -Wshadow -Woverloaded-virtual -Wno-deprecated -Wno-strict-overflow -Wno-array-bounds -Wunreachable-code -Wno-missing-field-initializers -Wno-unused-parameter -Wmissing-declarations -Wredundant-decls -Wpointer-arith -Werror)
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
cmake_dependent_option(F3D_DISABLE_DEFAULT_LIGHTS_TESTS_COMPARISON "Disable image comparison on tests using the default lights" OFF "BUILD_TESTING" ON)
cmake_dependent_option(F3D_ENABLE_LONG_TIMEOUT_TESTS "Enable long timeout tests" OFF "BUILD_TESTING" ON)
cmake_dependent_option(F3D_ENABLE_HDRI_TESTS "Enable HDRi related tests" ON "F3D_ENABLE_LONG_TIMEOUT_TESTS" ON "BUILD_TESTING" ON)
if(BUILD_TESTING)
  enable_testing()
endif()

# plugins
option(F3D_PLUGINS_STATIC_BUILD "Make all plugins static (embedded into libf3d) and automatically loaded by F3D" ON)
mark_as_advanced(F3D_PLUGINS_STATIC_BUILD)
add_subdirectory(plugins)

# libf3d target
add_subdirectory(library)

# f3d target
cmake_dependent_option(F3D_BUILD_APPLICATION "Build the application" ON "NOT ANDROID" OFF)
if (F3D_BUILD_APPLICATION)
  add_subdirectory(application)

  # applicative tests
  if(BUILD_TESTING)
    include(cmake/testing.cmake)
  endif()
endif()

# Windows Shell Extension
cmake_dependent_option(F3D_BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION "Build the Windows Shell Extension to produce thumbnails" ON "WIN32" OFF)
if(F3D_BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION)
  add_subdirectory(winshellext)
endif()

# Python bindings
option(F3D_BINDINGS_PYTHON "Create Python bindings" OFF)
if(F3D_BINDINGS_PYTHON)
  find_package(Python 3.6 COMPONENTS Interpreter Development)
  message(STATUS "Bindings: Python: Python ${Python_VERSION} found")
  add_subdirectory(python)
endif()

# JNI bindings
option(F3D_BINDINGS_JAVA "Create Java bindings" OFF)
if(F3D_BINDINGS_JAVA)
  add_subdirectory(java)
endif()

# Installation
option(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE "Install a default configuration file" OFF)
mark_as_advanced(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
if(UNIX AND NOT APPLE)
  cmake_dependent_option(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX "Install the default configuration at the prefix root instead of system wide" OFF
    "F3D_INSTALL_DEFAULT_CONFIGURATION_FILE" OFF)
  mark_as_advanced(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX)
endif()

if(WIN32)
  option(F3D_INSTALL_LOGOS_FOR_NSIS_PACKAGING "Install logos for nsis packaging" OFF)
  mark_as_advanced(F3D_INSTALL_LOGOS_FOR_NSIS_PACKAGING)
endif()

if(UNIX AND NOT APPLE)
  option(F3D_INSTALL_THUMBNAILER_FILES "Install thumbnailer files" OFF)
  mark_as_advanced(F3D_INSTALL_THUMBNAILER_FILES)

  option(F3D_INSTALL_MIME_TYPES_FILES "Install mime types files" OFF)
  mark_as_advanced(F3D_INSTALL_MIME_TYPES_FILES)
endif()

include(cmake/installing.cmake)

# packaging
include(cmake/packaging.cmake)
