cmake_minimum_required(VERSION 3.1)
include(CMakeDependentOption)

project(f3d
  VERSION "1.1.0"
  DESCRIPTION "F3D - A fast and minimalist 3D viewer"
  LANGUAGES C CXX)

string(TIMESTAMP F3D_BUILD_DATE "%Y-%m-%d %H:%M:%S")

# CMake variables
option(BUILD_TESTING "Build the tests" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

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
    jsoncpp
    opengl
  OPTIONAL_COMPONENTS
    IOExodus
    RenderingRayTracing)

if(${VTK_VERSION} VERSION_LESS 9.0)
  message(FATAL_ERROR "VTK >= 9.0 is REQUIRED")
endif()

# Modules
option(F3D_MODULE_RAYTRACING "Raytracing module" OFF)
option(F3D_MODULE_EXODUS "ExodusII module" ON)
option(F3D_MODULE_OCCT "OpenCASCADE module (STEP and IGES files)" OFF)

set(F3D_VTK_MODULES
  VTK::CommonSystem
  VTK::FiltersGeneral
  VTK::FiltersGeometry
  VTK::ImagingCore
  VTK::ImagingHybrid
  VTK::InteractionStyle
  VTK::InteractionWidgets
  VTK::IOCityGML
  VTK::IOGeometry
  VTK::IOImage
  VTK::IOImport
  VTK::IOParallel
  VTK::IOPLY
  VTK::IOXML
  VTK::RenderingAnnotation
  VTK::RenderingCore
  VTK::RenderingLabel
  VTK::RenderingOpenGL2
  VTK::RenderingVolumeOpenGL2
  VTK::jsoncpp
  VTK::opengl
)

if(F3D_MODULE_RAYTRACING)
  if(TARGET VTK::RenderingRayTracing)
    message(STATUS "VTK RayTracing enabled")
    set(F3D_VTK_MODULES ${F3D_VTK_MODULES} VTK::RenderingRayTracing)
  else()
    message(WARNING "F3D_MODULE_RAYTRACING option is enabled but VTK::RenderingRayTracing is not found")
    set(F3D_MODULE_RAYTRACING OFF)
  endif()
endif()

if(F3D_MODULE_EXODUS)
  if(TARGET VTK::IOExodus)
    message(STATUS "VTK ExodusII enabled")
    set(F3D_VTK_MODULES ${F3D_VTK_MODULES} VTK::IOExodus)
  else()
    message(WARNING "F3D_MODULE_EXODUS option is enabled but VTK::IOExodus is not found")
    set(F3D_MODULE_EXODUS OFF)
  endif()
endif()

# Build f3d
include(GNUInstallDirs)
add_subdirectory(src)

# testing
if(BUILD_TESTING)
  include(cmake/testing.cmake)
endif()

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
