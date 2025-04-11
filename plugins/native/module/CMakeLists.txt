set(classes
  vtkF3DSplatReader
  vtkF3DQuakeMDLImporter
  )

set(_no_install "")
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220928)
  set(_no_install "NO_INSTALL")
endif()

vtk_module_add_module(f3d::vtkextNative
  ${_no_install}
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_set_properties(f3d::vtkextNative CXX_STANDARD 17)
