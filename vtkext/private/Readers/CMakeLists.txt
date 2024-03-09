set(classes
  vtkF3DGenericImporter
  vtkF3DMemoryMesh
  vtkF3DPostProcessFilter
  )

if(F3D_MODULE_EXR)
  find_package(OpenEXR 3.0 REQUIRED)
  list(APPEND classes vtkF3DEXRReader)
endif()

vtk_module_add_module(f3d::VTKExtensionsReaders
  ${f3d_vtk_no_install}
  FORCE_STATIC
  CLASSES ${classes})

if(F3D_MODULE_EXR)
  vtk_module_link(f3d::VTKExtensionsReaders PRIVATE OpenEXR::OpenEXR)
endif()
