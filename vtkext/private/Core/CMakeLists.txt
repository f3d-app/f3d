configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vtkF3DConfigure.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkF3DConfigure.h"
  @ONLY)

set(classes
  F3DLog
  vtkF3DConsoleOutputWindow
  )

if(WIN32 AND F3D_WINDOWS_GUI)
  list(APPEND classes vtkF3DWin32OutputWindow)
endif()

if(ANDROID)
  list(APPEND classes vtkF3DAndroidLogOutputWindow)
endif()

vtk_module_add_module(f3d::VTKExtensionsCore
  ${f3d_vtk_no_install}
  FORCE_STATIC
  CLASSES ${classes})

if(MSVC)
  vtk_module_definitions(f3d::VTKExtensionsCore PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS)
endif()
