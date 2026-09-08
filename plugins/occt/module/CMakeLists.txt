set(classes
  vtkF3DOCCTReader
  vtkF3DFCStdReader
  )

set(_miniz_sources)
if(NOT F3D_USE_EXTERNAL_MINIZ)
  set(_miniz_sources "${F3D_SOURCE_DIR}/external/miniz/miniz.c")
endif()

vtk_module_add_module(f3d::vtkextOCCT
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes}
  SOURCES ${_miniz_sources})

vtk_module_include(f3d::vtkextOCCT
  PRIVATE
    $<BUILD_INTERFACE:${OpenCASCADE_INCLUDE_DIR}>)

if(F3D_USE_EXTERNAL_MINIZ)
  vtk_module_link(f3d::vtkextOCCT PRIVATE miniz::miniz)
else()
  vtk_module_include(f3d::vtkextOCCT
    PRIVATE
      $<BUILD_INTERFACE:${F3D_SOURCE_DIR}/external/miniz>)
endif()

set(_occt_link_libs "${OpenCASCADE_LIBRARIES}")
if(CMAKE_LINK_GROUP_USING_RESCAN_SUPPORTED)
  # Static linking of recursively dependent libs is very complex with OCCT, this simplify it
  set(_occt_link_libs "$<LINK_GROUP:RESCAN,${OpenCASCADE_LIBRARIES}>")
endif()

vtk_module_link(f3d::vtkextOCCT PRIVATE "${_occt_link_libs}")

vtk_module_set_properties(f3d::vtkextOCCT CXX_STANDARD 20)

if(F3D_PLUGIN_OCCT_COLORING_SUPPORT)
  vtk_module_definitions(f3d::vtkextOCCT
    PRIVATE F3D_PLUGIN_OCCT_XCAF)
endif()
