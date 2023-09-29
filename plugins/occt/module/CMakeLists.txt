set(classes
  vtkF3DOCCTReader
  )

set(_no_install "")
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220928)
  set(_no_install "NO_INSTALL")
endif()

vtk_module_add_module(f3d::VTKExtensionsOCCTReader
  ${_no_install}
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_include(f3d::VTKExtensionsOCCTReader
  PRIVATE
    $<BUILD_INTERFACE:${OpenCASCADE_INCLUDE_DIR}>)

vtk_module_link(f3d::VTKExtensionsOCCTReader
  PRIVATE
    ${OpenCASCADE_LIBRARIES})

if(F3D_PLUGIN_OCCT_COLORING_SUPPORT)
  vtk_module_definitions(f3d::VTKExtensionsOCCTReader
    PRIVATE F3D_PLUGIN_OCCT_XCAF)
endif()
