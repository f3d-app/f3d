set(classes
  vtkF3DIFCReader
  )

vtk_module_add_module(f3d::vtkextIFC
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})
