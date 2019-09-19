cmake_minimum_required(VERSION 3.1)

project(f3d CXX)

set(VTK_REQ_COMPONENTS
  FiltersGeneral
  InteractionStyle
  InteractionWidgets
  IOCityGML
  IOGeometry
  IOImage
  IOImport
  IOPLY
  IOXML
  RenderingAnnotation
  RenderingCore
  RenderingLabel
  RenderingOpenGL2
)

find_package(VTK COMPONENTS
  ${VTK_REQ_COMPONENTS}
  REQUIRED)

add_executable(f3d
  main.cxx
  ImporterFactory.cxx
  vtkF3DInteractor.cxx
  vtkF3DMetaReader.cxx
)

target_link_libraries(f3d PUBLIC ${VTK_LIBRARIES})
vtk_module_autoinit(TARGETS f3d MODULES ${VTK_LIBRARIES})
