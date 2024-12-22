set(classes
  vtkF3DUSDImporter
  )

vtk_module_add_module(f3d::vtkextUSD
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_link(f3d::vtkextUSD PRIVATE ${PXR_LIBRARIES})

vtk_module_include(f3d::vtkextUSD PRIVATE ${PXR_INCLUDE_DIRS})

vtk_module_set_properties(f3d::vtkextUSD CXX_STANDARD 17)
