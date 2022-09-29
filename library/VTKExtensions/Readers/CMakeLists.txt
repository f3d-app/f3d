set(classes
  vtkF3DGenericImporter
  vtkF3DPostProcessFilter
  vtkF3DMetaReader
  )

vtk_module_add_module(f3d::VTKExtensionsReaders
  ${libf3d_vtk_no_install}
  FORCE_STATIC
  CLASSES ${classes})
