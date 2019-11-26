
if(WIN32)
  set(WINDOWS_RC_FILE ${CMAKE_SOURCE_DIR}/resources/f3d.rc)
endif()

add_executable(f3d
  main.cxx
  vtkF3DGenericImporter.cxx
  vtkF3DInteractor.cxx
  vtkF3DMetaReader.cxx
  vtkF3DOpenGLGridMapper.cxx
  F3DLoader.cxx
  F3DLog.cxx
  F3DOptions.cxx
  F3DViewer.cxx
  ${WINDOWS_RC_FILE}
)

set_target_properties(f3d PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

target_include_directories(f3d
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(f3d
  PRIVATE VTK_VERSION_MAJOR=${VTK_VERSION_MAJOR}
  PRIVATE VTK_VERSION_MINOR=${VTK_VERSION_MINOR}
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # MacOS Bundle app
  option(MACOSX_BUILD_BUNDLE ON)
  if(MACOSX_BUILD_BUNDLE)
    set_target_properties(f3d PROPERTIES MACOSX_BUNDLE TRUE)
  endif()

  # Add logo icon
  set(MACOSX_BUNDLE_ICON_FILE logo.icns)
  set(f3d_ICON ${CMAKE_SOURCE_DIR}/resources/logo.icns)
  set_source_files_properties(${f3d_ICON} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")
  target_sources(f3d PRIVATE ${f3d_ICON})
endif()

if(WIN32)
  # Windows executable without console
  option(WINDOWS_BUILD_WIN32 ON)
  if(WINDOWS_BUILD_WIN32)
    set_target_properties(f3d PROPERTIES WIN32_EXECUTABLE TRUE)
    set(F3D_WIN32_APP ON)
  endif()
endif()

vtk_module_autoinit(
  TARGETS f3d
  MODULES
    VTK::FiltersGeneral
    VTK::FiltersGeometry
    VTK::ImagingCore
    VTK::InteractionStyle
    VTK::InteractionWidgets
    VTK::IOCityGML
    VTK::IOGeometry
    VTK::IOImage
    VTK::IOImport
    VTK::IOPLY
    VTK::IOXML
    VTK::RenderingAnnotation
    VTK::RenderingCore
    VTK::RenderingLabel
    VTK::RenderingOpenGL2
    VTK::jsoncpp
)

target_link_libraries(f3d
  PUBLIC
    VTK::FiltersGeneral
    VTK::FiltersGeometry
    VTK::ImagingCore
    VTK::InteractionStyle
    VTK::InteractionWidgets
    VTK::IOCityGML
    VTK::IOGeometry
    VTK::IOImage
    VTK::IOImport
    VTK::IOPLY
    VTK::IOXML
    VTK::RenderingAnnotation
    VTK::RenderingCore
    VTK::RenderingLabel
    VTK::RenderingOpenGL2
    VTK::jsoncpp
)

if(TARGET VTK::RenderingRayTracing)
  message(STATUS "VTK RayTracing enabled")
  set(F3D_HAS_RAYTRACING ON)
  vtk_module_autoinit(
    TARGETS f3d
    MODULES VTK::RenderingRayTracing
  )
  target_link_libraries(f3d
    PUBLIC VTK::RenderingRayTracing
  )
endif()

configure_file(Config.h.in
  "${CMAKE_CURRENT_BINARY_DIR}/Config.h" @ONLY)

install(TARGETS f3d
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION ".")
