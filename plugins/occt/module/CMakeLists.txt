set(classes
  vtkF3DOCCTReader
  )

vtk_module_add_module(f3d::VTKExtensionsOCCTReader
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_include(f3d::VTKExtensionsOCCTReader
  PRIVATE
    $<BUILD_INTERFACE:${OpenCASCADE_INCLUDE_DIR}>)

vtk_module_link(f3d::VTKExtensionsOCCTReader
  PRIVATE
    ${OpenCASCADE_LIBRARIES})

if(F3D_PLUGIN_OCCT_XCAF)
  vtk_module_definitions(f3d::VTKExtensionsOCCTReader
    PRIVATE F3D_PLUGIN_OCCT_XCAF)
endif()
