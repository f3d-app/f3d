set(classes
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
  CLASSES ${classes})
