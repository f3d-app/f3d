set(F3D_DEFAULT_HDRI "${F3D_SOURCE_DIR}/resources/defaultHDRI.png" CACHE STRING "Default HDRI when no HDRI file is provided")
mark_as_advanced(F3D_DEFAULT_HDRI)
include("f3dEmbed")
f3d_embed_file(
  INPUT "${F3D_DEFAULT_HDRI}"
  NAME F3DDefaultHDRI
  BINARY)

set(sources ${CMAKE_CURRENT_BINARY_DIR}/F3DDefaultHDRI.cxx)
set(private_headers F3DImguiStyle.h ${CMAKE_CURRENT_BINARY_DIR}/F3DDefaultHDRI.h)

if(F3D_MODULE_UI)
  set(F3D_FONT_FILE "${F3D_SOURCE_DIR}/resources/MonaspaceNeon-Regular.otf" CACHE STRING "Font used in UI widgets")
  mark_as_advanced(F3D_FONT_FILE)

  f3d_embed_file(
    INPUT "${F3D_FONT_FILE}"
    NAME F3DFontBuffer
    BINARY)

  list(APPEND sources ${CMAKE_CURRENT_BINARY_DIR}/F3DFontBuffer.cxx)
  list(APPEND private_headers ${CMAKE_CURRENT_BINARY_DIR}/F3DFontBuffer.h)

  set(F3D_DEFAULT_LOGO "${F3D_SOURCE_DIR}/resources/logo-black.png" CACHE STRING "logo")
  mark_as_advanced(F3D_DEFAULT_LOGO)

  f3d_embed_file(
    INPUT "${F3D_DEFAULT_LOGO}"
    NAME F3DDefaultLogo
    BINARY)

  list(APPEND sources ${CMAKE_CURRENT_BINARY_DIR}/F3DDefaultLogo.cxx)
  list(APPEND private_headers ${CMAKE_CURRENT_BINARY_DIR}/F3DDefaultLogo.h)
endif()

set(shader_files glsl/vtkF3DComputeDepthCS.glsl)

if(F3D_MODULE_UI)
  list(APPEND shader_files glsl/vtkF3DImguiFS.glsl glsl/vtkF3DImguiVS.glsl)
endif()

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
  F3DColoringInfoHandler
  vtkF3DCachedLUTTexture
  vtkF3DCachedSpecularTexture
  vtkF3DConsoleOutputWindow
  vtkF3DExternalRenderWindow
  vtkF3DGenericImporter
  vtkF3DHexagonalBokehBlurPass
  vtkF3DInteractorEventRecorder
  vtkF3DInteractorStyle
  vtkF3DMemoryMesh
  vtkF3DMetaImporter
  vtkF3DNoRenderWindow
  vtkF3DObjectFactory
  vtkF3DOpenGLGridMapper
  vtkF3DOverlayRenderPass
  vtkF3DPolyDataMapper
  vtkF3DPostProcessFilter
  vtkF3DRenderPass
  vtkF3DRenderer
  vtkF3DSolidBackgroundPass
  vtkF3DUIObserver
  vtkF3DUIActor
  vtkF3DUserRenderPass
  )

if (F3D_MODULE_UI)
  set(classes ${classes} vtkF3DImguiActor vtkF3DImguiConsole vtkF3DImguiObserver)
endif ()

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10675
if(NOT ANDROID AND NOT EMSCRIPTEN AND VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203)
  set(classes ${classes} vtkF3DPointSplatMapper)
endif()

if(NOT VTK_VERSION VERSION_GREATER_EQUAL 9.2.20220907)
  set(classes ${classes} vtkF3DOrientationMarkerWidget)
endif()

if(WIN32)
  list(APPEND classes vtkF3DWGLRenderWindow)
endif()

# In theory the test should be `if (VTK_USE_X)` but this variable is
# only exported starting from VTK 9.4
if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN AND NOT ANDROID)
  list(APPEND classes vtkF3DGLXRenderWindow)
endif()

if(VTK_OPENGL_HAS_EGL)
  list(APPEND classes vtkF3DEGLRenderWindow)
endif()

if(ANDROID)
  list(APPEND classes vtkF3DAndroidLogOutputWindow)
endif()

if(F3D_MODULE_EXR)
  find_package(OpenEXR 3.0.1 REQUIRED)
  list(APPEND classes vtkF3DEXRReader)
endif()

if(F3D_MODULE_UI AND NOT F3D_USE_EXTERNAL_IMGUI)
  list(APPEND sources
    "${F3D_SOURCE_DIR}/external/imgui/imgui.cpp"
    "${F3D_SOURCE_DIR}/external/imgui/imgui_demo.cpp"
    "${F3D_SOURCE_DIR}/external/imgui/imgui_draw.cpp"
    "${F3D_SOURCE_DIR}/external/imgui/imgui_tables.cpp"
    "${F3D_SOURCE_DIR}/external/imgui/imgui_widgets.cpp")
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

# exr
if(F3D_MODULE_EXR)
  vtk_module_link(f3d::vtkextPrivate PRIVATE OpenEXR::OpenEXR)
endif()

# imgui
if (F3D_MODULE_UI)
  vtk_module_definitions(f3d::vtkextPrivate PRIVATE F3D_MODULE_UI)
  if(F3D_USE_EXTERNAL_IMGUI)
    vtk_module_link(f3d::vtkextPrivate PRIVATE imgui::imgui)
  else()
    vtk_module_include(f3d::vtkextPrivate
      PRIVATE
        "${F3D_SOURCE_DIR}/external/imgui")
  endif ()
endif ()

# raytracing
if (F3D_MODULE_RAYTRACING)
  vtk_module_definitions(f3d::vtkextPrivate PRIVATE F3D_MODULE_RAYTRACING)
endif ()
