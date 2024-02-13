set(classes
  vtkF3DDracoReader
  )

# TODO: correct version
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240212)
  set(classes ${classes}
    vtkF3DGLTFDocumentLoader
    vtkF3DGLTFImporter
    vtkF3DGLTFReader
    )
endif()

set(_no_install "")
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220928)
  set(_no_install "NO_INSTALL")
endif()

vtk_module_add_module(f3d::VTKExtensionsDracoReader
  ${_no_install}
  FORCE_STATIC
  CLASSES ${classes})
