set(classes
  vtkF3DUSDImporter
  )

vtk_module_add_module(f3d::VTKExtensionsUSDReader
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_link(f3d::VTKExtensionsUSDReader PRIVATE ${PXR_LIBRARIES})

vtk_module_set_properties(f3d::VTKExtensionsUSDReader CXX_STANDARD 17)
