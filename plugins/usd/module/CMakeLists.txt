set(classes
  vtkF3DUSDImporter
  )

vtk_module_add_module(f3d::vtkextUSD
  FORCE_STATIC
  CLASSES ${classes})

list(GET PXR_LIBRARIES 0 _first_lib)
get_target_property(_target_type ${_first_lib} TYPE)
if(_target_type STREQUAL "STATIC_LIBRARY")
  vtk_module_link(f3d::vtkextUSD PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,${PXR_LIBRARIES}>)
else()
  vtk_module_link(f3d::vtkextUSD PRIVATE ${PXR_LIBRARIES})
endif()

vtk_module_include(f3d::vtkextUSD PRIVATE ${PXR_INCLUDE_DIRS})

vtk_module_set_properties(f3d::vtkextUSD CXX_STANDARD 20)
