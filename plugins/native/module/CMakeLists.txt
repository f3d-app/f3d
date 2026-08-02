set(classes
  vtkF3DImageImporter
  vtkF3DQuakeMDLImporter
  vtkF3DSPZReader
  vtkF3DSplatReader
  vtkF3DPLYReader
  vtkF3DLYSReader
  )

vtk_module_add_module(f3d::vtkextNative
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_set_properties(f3d::vtkextNative CXX_STANDARD 20)

target_include_directories(vtkextNative PRIVATE
  $<BUILD_INTERFACE:${F3D_SOURCE_DIR}/external/nlohmann_json>)
