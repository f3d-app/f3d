set(classes
  vtkF3DAssimpImporter
  )

vtk_module_add_module(f3d::vtkextAssimp
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_set_properties(f3d::vtkextAssimp CXX_STANDARD 17)
