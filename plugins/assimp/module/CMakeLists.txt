set(classes
  vtkF3DAssimpImporter
  )

vtk_module_add_module(f3d::VTKExtensionsAssimpReader
  FORCE_STATIC
  CLASSES ${classes})
