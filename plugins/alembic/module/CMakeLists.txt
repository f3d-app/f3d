set(classes
  vtkF3DAlembicReader
  )

vtk_module_add_module(f3d::vtkextAlembic
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})
