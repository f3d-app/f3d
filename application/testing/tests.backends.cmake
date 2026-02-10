## Test rendering backends

f3d_application_test(NAME TestRenderingBackendAuto DATA cow.vtp RENDERING_BACKEND auto)
f3d_application_test(NAME TestRenderingBackendInvalid DATA cow.vtp RENDERING_BACKEND invalid ARGS --verbose REGEXP "rendering-backend value is invalid, falling back to" NO_BASELINE)

if(F3D_TESTING_ENABLE_OSMESA_TESTS)
  f3d_application_test(NAME TestRenderingBackenListOSMesa ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "osmesa: available")
endif()

if(WIN32)
  f3d_application_test(NAME TestRenderingBackendWGL DATA cow.vtp RENDERING_BACKEND wgl)
  f3d_application_test(NAME TestRenderingBackendWGLCheckClass DATA cow.vtp RENDERING_BACKEND wgl ARGS --verbose REGEXP "vtkF3DWGLRenderWindow" NO_BASELINE)
  f3d_application_test(NAME TestRenderingBackendWindowsAutoCheckClass DATA cow.vtp ARGS --verbose REGEXP "vtkF3DWGLRenderWindow" NO_BASELINE)
  f3d_application_test(NAME TestRenderingBackendGLXFailure DATA cow.vtp RENDERING_BACKEND glx REGEXP "Cannot use a GLX context on this platform" NO_BASELINE)
  f3d_application_test(NAME TestRenderingBackenListWGL ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "wgl: available")
elseif(APPLE)
  f3d_application_test(NAME TestRenderingBackenListCOCOA ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "cocoa: available")
endif()

if (VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240914)

  if(F3D_TESTING_ENABLE_GLX_TESTS)
    f3d_application_test(NAME TestRenderingBackenListGLX ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "glx: available")
  endif()

  if(F3D_TESTING_ENABLE_EGL_TESTS)
    f3d_application_test(NAME TestRenderingBackenListEGL ARGS --list-rendering-backends NO_RENDER NO_BASELINE REGEXP "egl: available")
  endif()

  # For some reason the sanitizer detects leaks because of EGL and OSMesa
  if(UNIX AND NOT APPLE AND F3D_SANITIZER STREQUAL "none")

    f3d_application_test(NAME TestRenderingBackendWGLFailure DATA cow.vtp RENDERING_BACKEND wgl REGEXP "Cannot use a WGL context on this platform" NO_BASELINE)

    if(F3D_TESTING_ENABLE_EGL_TESTS)
      f3d_application_test(NAME TestRenderingBackendEGL DATA cow.vtp RENDERING_BACKEND egl)
      f3d_application_test(NAME TestRenderingBackendEGLCheckClass DATA cow.vtp RENDERING_BACKEND egl ARGS --verbose REGEXP "vtkF3DEGLRenderWindow" NO_BASELINE)
      f3d_application_test(NAME TestRenderingBackendEGLNoInteractiveVerbose RENDERING_BACKEND egl REGEXP "You are using an offscreen configuration, interactive rendering is disabled" NO_BASELINE NO_OUTPUT)
    endif()

    if(F3D_TESTING_ENABLE_OSMESA_TESTS)
      f3d_application_test(NAME TestRenderingBackendOSMesa DATA cow.vtp RENDERING_BACKEND osmesa)
      f3d_application_test(NAME TestRenderingBackendOSMesaCheckClass DATA cow.vtp RENDERING_BACKEND osmesa ARGS --verbose REGEXP "vtkOSOpenGLRenderWindow" NO_BASELINE)
      f3d_application_test(NAME TestRenderingBackendOSMesaNoInteractiveVerbose RENDERING_BACKEND osmesa REGEXP "You are using an offscreen configuration, interactive rendering is disabled" NO_BASELINE NO_OUTPUT)
    endif()

    if(F3D_TESTING_ENABLE_GLX_TESTS)
      f3d_application_test(NAME TestRenderingBackendGLX DATA cow.vtp RENDERING_BACKEND glx)
      f3d_application_test(NAME TestRenderingBackendGLXNoDisplay DATA cow.vtp RENDERING_BACKEND glx ENV "DISPLAY=:7" REGEXP "Cannot create a window" NO_BASELINE)
      f3d_application_test(NAME TestRenderingBackendGLXCheckClass DATA cow.vtp RENDERING_BACKEND glx ARGS --verbose REGEXP "vtkF3DGLXRenderWindow" NO_BASELINE)
      f3d_application_test(NAME TestRenderingBackendLinuxAutoCheckClass DATA cow.vtp ARGS --verbose REGEXP "vtkF3DGLXRenderWindow" NO_BASELINE)
    endif()
  endif()
endif()
