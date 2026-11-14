set(classes
  vtkF3DImageImporter
  vtkF3DQuakeMDLImporter
  vtkF3DSPZReader
  vtkF3DSplatReader
  vtkF3DPLYReader
  )

if(F3D_MODULE_WEBP)
  list(APPEND classes vtkF3DSOGReader)
endif()

vtk_module_add_module(f3d::vtkextNative
  NO_INSTALL
  FORCE_STATIC
  CLASSES ${classes})

vtk_module_set_properties(f3d::vtkextNative CXX_STANDARD 20)

if(F3D_MODULE_WEBP)
  if(F3D_USE_EXTERNAL_NLOHMANN_JSON)
    vtk_module_link(f3d::vtkextNative PRIVATE nlohmann_json::nlohmann_json)
  else()
    vtk_module_include(f3d::vtkextNative
      PRIVATE
        $<BUILD_INTERFACE:${F3D_SOURCE_DIR}/external/nlohmann_json>)
  endif()
endif()
