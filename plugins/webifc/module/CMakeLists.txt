set(classes
  vtkF3DWebIFCReader
  )

vtk_module_add_module(f3d::vtkextWebIFC
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})
