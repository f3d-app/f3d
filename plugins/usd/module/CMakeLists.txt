set(classes
  vtkF3DFaceVaryingPointDispatcher
  vtkF3DUSDImporter
  )

vtk_module_add_module(f3d::VTKExtensionsUSDReader
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_link(f3d::VTKExtensionsUSDReader PRIVATE ${PXR_LIBRARIES})

# When built in monolithic mode, include must be specified manually here because
# the "usd_ms" target does not propagate the directories automatically
find_package(Boost REQUIRED)

vtk_module_include(f3d::VTKExtensionsUSDReader PRIVATE ${Boost_INCLUDE_DIRS} ${PXR_INCLUDE_DIRS})

vtk_module_set_properties(f3d::VTKExtensionsUSDReader CXX_STANDARD 17)
