set(classes
  vtkF3DObjectFactory
  )

vtk_module_add_module(f3d::VTKExtensionsApplicative
  ${libf3d_vtk_no_install}
  FORCE_STATIC
  CLASSES ${classes})
