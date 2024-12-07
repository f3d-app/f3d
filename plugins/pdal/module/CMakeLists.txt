set(classes
  vtkF3DPDALReader
  )

vtk_module_add_module(f3d::vtkF3DPDALReader
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_link(f3d::vtkF3DPDALReader PRIVATE ${PXR_LIBRARIES})

vtk_module_include(f3d::vtkF3DPDALReader PRIVATE ${PXR_INCLUDE_DIRS})

vtk_module_set_properties(f3d::vtkF3DPDALReader CXX_STANDARD 17)