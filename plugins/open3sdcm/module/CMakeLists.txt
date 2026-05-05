set(classes
  vtkF3DOpen3SDCMImporter
  )

# Add Open3SDCM include directory to the VTK module
# Use absolute path since Open3SDCM_SOURCE_DIR may not be available in this scope
include_directories(/Users/romainnosenzo/CLionProjects/Open3SDCM/Lib/src)

vtk_module_add_module(f3d::vtkextOpen3SDCM
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_set_properties(f3d::vtkextOpen3SDCM CXX_STANDARD 20)
