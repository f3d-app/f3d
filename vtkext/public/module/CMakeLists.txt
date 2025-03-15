set(shader_files
  glsl/vtkF3DBitonicSortGlobalDisperseCS.glsl
  glsl/vtkF3DBitonicSortGlobalFlipCS.glsl
  glsl/vtkF3DBitonicSortLocalDisperseCS.glsl
  glsl/vtkF3DBitonicSortLocalSortCS.glsl
  glsl/vtkF3DBitonicSortFunctions.glsl)

foreach(file IN LISTS shader_files)
  vtk_encode_string(
    INPUT         "${file}"
    HEADER_OUTPUT header
    SOURCE_OUTPUT source)
  list(APPEND sources
    "${source}")
  list(APPEND private_headers
    "${header}")
endforeach()

set(classes
  F3DUtils
  vtkF3DFaceVaryingPointDispatcher
  vtkF3DGLTFImporter
  vtkF3DImporter
  )

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10675
if(NOT ANDROID AND NOT EMSCRIPTEN AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203)
  set(classes ${classes} vtkF3DBitonicSort)
endif()

vtk_module_add_module(f3d::vtkext
  ${f3d_vtk_no_install}
  ${f3d_vtk_force_static}
  CLASSES ${classes}
  SOURCES ${sources}
  PRIVATE_HEADERS ${private_headers}
  )
