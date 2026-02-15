set(classes
  vtkF3DImageImporter
  vtkF3DQuakeMDLImporter
  vtkF3DSPZReader
  vtkF3DSplatReader
  vtkF3DPLYReader
  )

vtk_module_add_module(f3d::vtkextNative
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_set_properties(f3d::vtkextNative CXX_STANDARD 17)

if (F3D_MODULE_EXR)
  target_compile_definitions(f3d::vtkextNative PRIVATE F3D_MODULE_EXR)
endif ()

if (F3D_MODULE_WEBP)
  target_compile_definitions(f3d::vtkextNative PRIVATE F3D_MODULE_WEBP)
endif ()
