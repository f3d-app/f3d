set(classes
  vtkF3DAlembicImporter
  )

vtk_module_add_module(f3d::VTKExtensionsAlembicReader
  FORCE_STATIC
  CLASSES ${classes})
