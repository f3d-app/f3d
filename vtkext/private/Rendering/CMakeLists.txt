set(F3D_DEFAULT_HDRI "${F3D_SOURCE_DIR}/resources/defaultHDRI.png" CACHE STRING "Default HDRI when no HDRI file is provided")
mark_as_advanced(F3D_DEFAULT_HDRI)
include("f3dEmbed")
f3d_embed_file(
  INPUT "${F3D_DEFAULT_HDRI}"
  NAME F3DDefaultHDRI
  BINARY)
set(sources
  ${CMAKE_CURRENT_BINARY_DIR}/F3DDefaultHDRI.cxx)
set(private_headers
  ${CMAKE_CURRENT_BINARY_DIR}/F3DDefaultHDRI.h)

set(shader_files
  glsl/vtkF3DComputeDepthCS.glsl)

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
  vtkF3DCachedLUTTexture
  vtkF3DCachedSpecularTexture
  vtkF3DDropZoneActor
  vtkF3DHexagonalBokehBlurPass
  vtkF3DInteractorEventRecorder
  vtkF3DInteractorStyle
  vtkF3DNoRenderWindow
  vtkF3DOpenGLGridMapper
  vtkF3DPolyDataMapper
  vtkF3DRenderPass
  vtkF3DRenderer
  vtkF3DRendererWithColoring
  )

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10675
if(NOT ANDROID AND NOT EMSCRIPTEN AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203)
  set(classes ${classes} vtkF3DPointSplatMapper)
endif()

if(NOT VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220907)
  set(classes ${classes} vtkF3DOrientationMarkerWidget)
endif()

vtk_module_add_module(f3d::VTKExtensionsRendering
  ${f3d_vtk_no_install}
  FORCE_STATIC
  CLASSES ${classes}
  SOURCES ${sources}
  PRIVATE_HEADERS ${private_headers}
  )
