set(classes
  vtkF3DSPZReader
  )

set(_no_install "")
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220928)
  set(_no_install "NO_INSTALL")
endif()

vtk_module_add_module(f3d::vtkextSPZ
  ${_no_install}
  FORCE_STATIC
  CLASSES ${classes})

# TODO: fix upstream
vtk_module_set_properties(f3d::vtkextSPZ CXX_STANDARD 20)

if (MSVC)
  # TODO: fix upstream
  vtk_module_definitions(f3d::vtkextSPZ PRIVATE _USE_MATH_DEFINES)
endif()
