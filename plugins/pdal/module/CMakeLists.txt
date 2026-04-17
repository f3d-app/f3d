set(classes
  vtkF3DPDALReader
  )

vtk_module_add_module(f3d::vtkextPDAL
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_set_properties(f3d::vtkextPDAL CXX_STANDARD 20)
