set(classes
  vtkF3DUSDImporter
  )

vtk_module_add_module(f3d::vtkextUSD
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_link(f3d::vtkextUSD PRIVATE ${PXR_LIBRARIES})

# When built in monolithic mode, include must be specified manually here because
# the "usd_ms" target does not propagate the directories automatically
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
find_package(Boost REQUIRED)

vtk_module_include(f3d::vtkextUSD PRIVATE ${Boost_INCLUDE_DIRS} ${PXR_INCLUDE_DIRS})

vtk_module_set_properties(f3d::vtkextUSD CXX_STANDARD 17)
