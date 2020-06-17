cmake_minimum_required(VERSION 3.1)
include(CMakeDependentOption)

project(f3d
  VERSION "1.0.0"
  DESCRIPTION "F3D - A fast and minimalist 3D viewer"
  LANGUAGES CXX)

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
    IOExodus
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
    RenderingRayTracing
)

if(${VTK_VERSION} VERSION_LESS 9.0)
  message(FATAL_ERROR "VTK >= 9.0 is REQUIRED")
endif()

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
include(cmake/installing.cmake)

# packaging
include(cmake/packaging.cmake)
