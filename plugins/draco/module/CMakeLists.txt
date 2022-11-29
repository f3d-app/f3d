set(classes
  vtkF3DDracoReader
  )

vtk_module_add_module(f3d::VTKExtensionsDracoReader
  FORCE_STATIC
  CLASSES ${classes})
