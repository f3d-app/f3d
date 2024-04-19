set(classes
  vtkF3DOCCTReader
  )

set(_no_install "")
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220928)
  set(_no_install "NO_INSTALL")
endif()

vtk_module_add_module(f3d::vtkextOCCT
  ${_no_install}
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_include(f3d::vtkextOCCT
  PRIVATE
    $<BUILD_INTERFACE:${OpenCASCADE_INCLUDE_DIR}>)

set(_occt_link_libs "${OpenCASCADE_LIBRARIES}")
if(CMAKE_LINK_GROUP_USING_RESCAN_SUPPORTED)
  # Static linking of recursively dependent libs is very complex with OCCT, this simplify it
  set(_occt_link_libs "$<LINK_GROUP:RESCAN,${OpenCASCADE_LIBRARIES}>")
endif()

vtk_module_link(f3d::vtkextOCCT PRIVATE "${_occt_link_libs}")

if(F3D_PLUGIN_OCCT_COLORING_SUPPORT)
  vtk_module_definitions(f3d::vtkextOCCT
    PRIVATE F3D_PLUGIN_OCCT_XCAF)
endif()
