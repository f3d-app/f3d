set(classes
  vtkF3DDracoReader
  vtkF3DGLTFDracoDocumentLoader
  vtkF3DGLTFDracoImporter
  )

vtk_module_add_module(f3d::vtkextDraco
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_set_properties(f3d::vtkextDraco CXX_STANDARD 17)
