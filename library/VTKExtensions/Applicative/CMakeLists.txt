set(classes
  vtkF3DObjectFactory
  )

vtk_module_add_module(f3d::VTKExtensionsApplicative
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})
