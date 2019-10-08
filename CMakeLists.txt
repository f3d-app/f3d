cmake_minimum_required(VERSION 3.1)

project(f3d CXX)

# CMake variables
option(BUILD_TESTING OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

set(VTK_REQUIRED_VERSION "8.2.0" CACHE STRING "VTK target version")
set_property(CACHE VTK_REQUIRED_VERSION PROPERTY STRINGS "8.2.0" "8.90.0")

# VTK dependency
find_package(VTK ${VTK_REQUIRED_VERSION} EXACT COMPONENTS
  vtkFiltersGeneral
  vtkFiltersGeometry
  vtkImagingCore
  vtkInteractionStyle
  vtkInteractionWidgets
  vtkIOCityGML
  vtkIOGeometry
  vtkIOImage
  vtkIOImport
  vtkIOPLY
  vtkIOXML
  vtkRenderingAnnotation
  vtkRenderingCore
  vtkRenderingLabel
  vtkRenderingOpenGL2
  vtkjsoncpp
  QUIET
  REQUIRED)

add_executable(f3d
  main.cxx
  vtkF3DGenericImporter.cxx
  vtkF3DInteractor.cxx
  vtkF3DMetaReader.cxx
  vtkF3DOpenGLGridMapper.cxx
  F3DOptions.cxx
  F3DViewer.cxx
)

target_compile_definitions(f3d
  PRIVATE VTK_VERSION_MAJOR=${VTK_VERSION_MAJOR}
  PRIVATE VTK_VERSION_MINOR=${VTK_VERSION_MINOR}
)

if(${VTK_VERSION} VERSION_LESS 8.90.0)
  include(${VTK_USE_FILE})
  target_link_libraries(f3d PUBLIC ${VTK_LIBRARIES})
else()
  target_link_libraries(f3d PUBLIC ${VTK_LIBRARIES})
  vtk_module_autoinit(TARGETS f3d MODULES ${VTK_LIBRARIES})
endif()

if(BUILD_TESTING)
  enable_testing()
  target_compile_definitions(f3d
    PRIVATE F3D_TESTING=1
  )

  function(f3d_test name input args)
    separate_arguments(args)
    add_test(NAME ${name}
             COMMAND ${CMAKE_BUILD_DIR}/f3d
               ${args}
               --ref ${CMAKE_SOURCE_DIR}/data/baselines/${name}.png
               ${CMAKE_SOURCE_DIR}/data/${input})
  endfunction()

  f3d_test(TestPLY suzanne.ply "--resolution 300,300")
  f3d_test(TestOBJ suzanne.obj "--resolution 300,300")
  f3d_test(TestSTL suzanne.stl "--resolution 300,300")
  f3d_test(TestVTU dragon.vtu "--resolution 300,300")
  f3d_test(TestGrid suzanne.ply "-g --resolution 300,300")
  f3d_test(TestAxis suzanne.ply "-x --resolution 300,300")
  f3d_test(TestPointCloud pointsCloud.vtp "--resolution 300,300")
endif()
