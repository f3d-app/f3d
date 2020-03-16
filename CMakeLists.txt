cmake_minimum_required(VERSION 3.1)

project(f3d
  VERSION "0.2.0"
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
    jsoncpp
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
include(cmake/installing.cmake)

# packaging
include(cmake/packaging.cmake)
