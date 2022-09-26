set(classes
  vtkF3DInteractorEventRecorder
  vtkF3DInteractorStyle2D
  vtkF3DInteractorStyle3D
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
  FORCE_STATIC
  CLASSES ${classes})
