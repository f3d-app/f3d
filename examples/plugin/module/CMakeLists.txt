set(classes
  vtkF3DExampleReader
  )

vtk_module_add_module(f3d::VTKExtensionsExampleReader
  FORCE_STATIC
  CLASSES ${classes})
