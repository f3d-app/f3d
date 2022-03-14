set(classes
  vtkF3DInteractorEventRecorder
  vtkF3DInteractorStyle
  vtkF3DOpenGLGridMapper
  vtkF3DPolyDataMapper
  vtkF3DRenderPass
  vtkF3DRenderer
  vtkF3DRendererWithColoring
  )

vtk_module_add_module(f3d::VTKExtensionsRendering
  FORCE_STATIC
  CLASSES ${classes})
