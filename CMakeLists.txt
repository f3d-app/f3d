cmake_minimum_required(VERSION 3.1)
include(CMakeDependentOption)

project(f3d
  VERSION "1.2.1"
  DESCRIPTION "F3D - A fast and minimalist 3D viewer"
  LANGUAGES C CXX)

string(TIMESTAMP F3D_BUILD_DATE "%Y-%m-%d %H:%M:%S" UTC)

# CMake variables
option(BUILD_TESTING "Build the tests" OFF)
cmake_dependent_option(F3D_DISABLE_DEFAULT_LIGHTS_TESTS_COMPARISON "Disable image comparison on tests using the default lights" OFF "BUILD_TESTING" ON)
cmake_dependent_option(F3D_ENABLE_LONG_TIMEOUT_TESTS "Enable long timeout tests" OFF "BUILD_TESTING" ON)
cmake_dependent_option(F3D_ENABLE_HDRI_TESTS "Enable HDRi related tests" ON "F3D_ENABLE_LONG_TIMEOUT_TESTS" ON "BUILD_TESTING" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

option(F3D_INSTALL_SDK "Install F3D library SDK" OFF)

option(F3D_GENERATE_MAN "Generate and install a man entry for f3d using help2man" OFF)
mark_as_advanced(F3D_GENERATE_MAN)

# Modules
option(F3D_MODULE_RAYTRACING "Raytracing module" OFF)
option(F3D_MODULE_EXODUS "ExodusII module" ON)
option(F3D_MODULE_OCCT "OpenCASCADE module (STEP and IGES files)" OFF)
option(F3D_MODULE_ASSIMP "Assimp module (FBX, OFF, DAE anf DXF files)" OFF)
option(F3D_MODULE_ALEMBIC "Alembic module (ABC files)" OFF)

# VTK dependency
find_package(VTK 9.0 REQUIRED
  COMPONENTS
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
    RenderingRayTracing)

# Windows Shell Extension DLL
cmake_dependent_option(BUILD_WINDOWS_SHELL_THUMBNAILS_EXTENSION "Build the Windows Shell Extension to produce thumbnails" ON "WIN32" OFF)

# testing
if(BUILD_TESTING)
  enable_testing()
  include(cmake/testing.cmake)
endif()

# Build f3d
include(GNUInstallDirs)
add_subdirectory(src)

# installation
option(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE "Install a default configuration file" OFF)
mark_as_advanced(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE)
if(UNIX AND NOT APPLE)
  cmake_dependent_option(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX "Install the default configuration at the prefix root instead of system wide" OFF
    "F3D_INSTALL_DEFAULT_CONFIGURATION_FILE" OFF)
  mark_as_advanced(F3D_INSTALL_DEFAULT_CONFIGURATION_FILE_IN_PREFIX)
endif()

if(UNIX AND NOT APPLE)
  option(F3D_INSTALL_THUMBNAILER_FILES "Install thumbnailer files" OFF)
  mark_as_advanced(F3D_INSTALL_THUMBNAILER_FILES)

  option(F3D_INSTALL_MIME_TYPES_FILE "Install mime types files" OFF)
  mark_as_advanced(F3D_INSTALL_MIME_TYPES_FILE)
endif()

include(cmake/installing.cmake)

# packaging
include(cmake/packaging.cmake)
