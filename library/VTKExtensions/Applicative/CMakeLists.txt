set(classes
  vtkF3DObjectFactory
  )

vtk_module_add_module(f3d::VTKExtensionsApplicative
  FORCE_STATIC
  CLASSES ${classes})
