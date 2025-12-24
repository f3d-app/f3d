set(classes
  vtkF3DAlembicReader
  )

vtk_module_add_module(f3d::vtkextAlembic
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_set_properties(f3d::vtkextAlembic CXX_STANDARD 17)
