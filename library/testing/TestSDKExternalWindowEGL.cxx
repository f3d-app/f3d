#include "engine.h"

#include "TestSDKHelpers.h"

#include <EGL/egl.h>

int TestSDKExternalWindowEGL(int argc, char* argv[])
{
  EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  // initialize
  EGLint major, minor;
  eglInitialize(eglDpy, &major, &minor);

  // configure
  EGLint numConfigs;
  EGLConfig eglCfg;

  constexpr EGLint configAttribs[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 8, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE };
  eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);

  // create a surface
  constexpr int size[] = { 300, 300 };
  constexpr EGLint pbufferAttribs[] = { EGL_WIDTH, size[0], EGL_HEIGHT, size[1], EGL_NONE };
  EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, pbufferAttribs);

  // bind the API
  eglBindAPI(EGL_OPENGL_API);

  // create a context
  EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, nullptr);
  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);

  f3d::engine eng = f3d::engine::createExternalEGL();
  eng.getWindow().setSize(size[0], size[1]);
  eng.getScene().add(std::string(argv[1]) + "/data/cow.vtp");

  if (!TestSDKHelpers::RenderTest(
        eng.getWindow(), std::string(argv[1]) + "baselines/", argv[2], "TestSDKExternalWindowEGL"))
  {
    return EXIT_FAILURE;
  }

  // terminate EGL when finished
  eglTerminate(eglDpy);
  return 0;
}
