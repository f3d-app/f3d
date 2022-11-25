set(classes
  vtkF3DExampleReader
  )

vtk_module_add_module(example::VTKExtensionsExampleReader
  FORCE_STATIC
  CLASSES ${classes})
