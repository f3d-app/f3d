set(classes
  vtkF3DObjectFactory
  )

vtk_module_add_module(f3d::VTKExtensionsApplicative
  ${f3d_vtk_no_install}
  FORCE_STATIC
  CLASSES ${classes})
