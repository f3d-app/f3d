cmake_minimum_required(VERSION 3.1)

message(STATUS "env: $ENV{CMAKE_GENERATOR}, var: ${CMAKE_GENERATOR}")

project(f3d
  VERSION "0.1.0"
  DESCRIPTION "f3d - Fast 3D Viewer"
  LANGUAGES CXX)

# CMake variables
option(BUILD_TESTING OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# VTK dependency
find_package(VTK 8.90.0 COMPONENTS
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
  
message(STATUS "VTK version: ${VTK_VERSION}")

if(${VTK_VERSION} VERSION_LESS 8.90.0)
  message(FATAL_ERROR "VTK >= 9.0 is REQUIRED")
endif()

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

target_link_libraries(f3d PUBLIC ${VTK_LIBRARIES})
vtk_module_autoinit(TARGETS f3d MODULES ${VTK_LIBRARIES})

if(BUILD_TESTING)
  enable_testing()

  function(f3d_test name input args)
    separate_arguments(args)
    add_test(NAME ${name}
             COMMAND ${CMAKE_BUILD_DIR}/f3d
               ${args}
               --ref ${CMAKE_SOURCE_DIR}/data/baselines/${name}.png
               ${CMAKE_SOURCE_DIR}/data/${input})
  endfunction()

  f3d_test(TestPLY suzanne.ply "-p --resolution 300,300")
  f3d_test(TestOBJ suzanne.obj "-p --resolution 300,300")
  f3d_test(TestSTL suzanne.stl "-p --resolution 300,300")
  f3d_test(TestVTU dragon.vtu "-p --resolution 300,300")
  f3d_test(TestGrid suzanne.ply "-pg --resolution 300,300")
  f3d_test(TestAxis suzanne.ply "-px --resolution 300,300")
  f3d_test(TestPointCloud pointsCloud.vtp "-p --resolution 300,300")
  f3d_test(TestVRMLImporter bot2.wrl "-pi --resolution 300,300")
  f3d_test(Test3DSImporter iflamigm.3ds "-pi --resolution 300,300")
  f3d_test(TestOBJImporter world.obj "-pi --resolution 300,300")
  f3d_test(TestScalars suzanne.ply "-pb --scalars=Normals --comp=1 --resolution 300,300")
  f3d_test(TestScalarsWithBar suzanne.ply "-p --scalars=Normals --comp=0 --resolution 300,300")
  if(${VTK_VERSION} VERSION_GREATER 8.2.0)
    f3d_test(TestGLTFImporter WaterBottle.glb "-pi --resolution 300,300")
  endif()
endif()

# installation
include(GNUInstallDirs)
install(TARGETS f3d
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# packaging
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CPACK_GENERATOR NSIS64 ZIP)
  set(CPACK_NSIS_MODIFY_PATH ON)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CPACK_GENERATOR TGZ TXZ)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CPACK_GENERATOR DMG PKG)
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Kitware SAS")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
include(CPack)
