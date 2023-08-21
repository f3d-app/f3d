include("f3dEmbed")
f3d_embed_file(
  INPUT "${F3D_SOURCE_DIR}/resources/defaultHDRI.png"
  NAME F3DDefaultHDRI
  BINARY)
set(sources
  ${CMAKE_CURRENT_BINARY_DIR}/F3DDefaultHDRI.cxx)
set(private_headers
  ${CMAKE_CURRENT_BINARY_DIR}/F3DDefaultHDRI.h)

set(classes
  vtkF3DCachedLUTTexture
  vtkF3DCachedSpecularTexture
  vtkF3DDropZoneActor
  vtkF3DHexagonalBokehBlurPass
  vtkF3DInteractorEventRecorder
  vtkF3DInteractorStyle
  vtkF3DNoRenderWindow
  vtkF3DOpenGLGridMapper
  vtkF3DPolyDataMapper
  vtkF3DRenderPass
  vtkF3DRenderer
  vtkF3DRendererWithColoring
  )

if(NOT VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220907)
    set(classes ${classes} vtkF3DOrientationMarkerWidget)
endif()

vtk_module_add_module(f3d::VTKExtensionsRendering
  ${libf3d_vtk_no_install}
  FORCE_STATIC
  CLASSES ${classes}
  SOURCES ${sources}
  PRIVATE_HEADERS ${private_headers}
  )
