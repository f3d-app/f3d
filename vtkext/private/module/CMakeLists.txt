configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DConfigure.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkF3DConfigure.h"
  @ONLY)

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
  F3DLog
  vtkF3DCachedLUTTexture
  vtkF3DCachedSpecularTexture
  vtkF3DConsoleOutputWindow
  vtkF3DDropZoneActor
  vtkF3DGenericImporter
  vtkF3DHexagonalBokehBlurPass
  vtkF3DInteractorEventRecorder
  vtkF3DInteractorStyle
  vtkF3DMemoryMesh
  vtkF3DNoRenderWindow
  vtkF3DObjectFactory
  vtkF3DOpenGLGridMapper
  vtkF3DPolyDataMapper
  vtkF3DPostProcessFilter
  vtkF3DRenderPass
  vtkF3DRenderer
  vtkF3DRendererWithColoring
  vtkF3DUserRenderPass
  )

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10675
if(NOT ANDROID AND NOT EMSCRIPTEN AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203)
  set(classes ${classes} vtkF3DPointSplatMapper)
endif()

if(NOT VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220907)
  set(classes ${classes} vtkF3DOrientationMarkerWidget)
endif()

if(WIN32 AND F3D_WINDOWS_GUI)
  list(APPEND classes vtkF3DWin32OutputWindow)
endif()

if(ANDROID)
  list(APPEND classes vtkF3DAndroidLogOutputWindow)
endif()

if(F3D_MODULE_EXR)
  find_package(OpenEXR 3.0 REQUIRED)
  list(APPEND classes vtkF3DEXRReader)
endif()

set(_no_install "")
if(VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220928)
  set(_no_install "NO_INSTALL")
endif()

vtk_module_add_module(f3d::vtkextPrivate
  ${_no_install}
  FORCE_STATIC
  CLASSES ${classes}
  SOURCES ${sources}
  PRIVATE_HEADERS ${private_headers}
  )

if(MSVC)
  vtk_module_definitions(f3d::vtkextPrivate PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS)
endif()

if(F3D_MODULE_EXR)
  vtk_module_link(f3d::vtkextPrivate PRIVATE OpenEXR::OpenEXR)
endif()
