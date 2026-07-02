#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include "engine.h"

#include <EGL/egl.h>

int TestSDKExternalWindowEGL([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  // ensure we have a display
  if (eglDpy == EGL_NO_DISPLAY)
  {
    std::cerr << "Skipping test: no EGL display available\n";
    return EXIT_FAILURE;
  }

  // initialize
  EGLint major, minor;
  if (eglInitialize(eglDpy, &major, &minor) == EGL_FALSE)
  {
    std::cerr << "Skipping test: eglInitialize failed\n";
    return EXIT_FAILURE;
  }

  // configure
  EGLint numConfigs;
  EGLConfig eglCfg;

  constexpr EGLint configAttribs[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 8, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE };
  if (eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs) == EGL_FALSE ||
    numConfigs <= 0)
  {
    std::cerr << "Skipping test: eglChooseConfig failed or returned no configs\n";
    eglTerminate(eglDpy);
    return EXIT_FAILURE;
  }

  // create a surface
  constexpr int size[] = { 300, 300 };
  constexpr EGLint pbufferAttribs[] = { EGL_WIDTH, size[0], EGL_HEIGHT, size[1], EGL_NONE };
  EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, pbufferAttribs);
  if (eglSurf == EGL_NO_SURFACE)
  {
    std::cerr << "Skipping test: eglCreatePbufferSurface failed\n";
    eglTerminate(eglDpy);
    return EXIT_FAILURE;
  }

  // bind the API
  if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE)
  {
    std::cerr << "Skipping test: eglBindAPI failed\n";
    eglDestroySurface(eglDpy, eglSurf);
    eglTerminate(eglDpy);
    return EXIT_FAILURE;
  }

  // create a context
  EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, nullptr);
  if (eglCtx == EGL_NO_CONTEXT)
  {
    std::cerr << "Skipping test: eglCreateContext failed\n";
    eglDestroySurface(eglDpy, eglSurf);
    eglTerminate(eglDpy);
    return EXIT_FAILURE;
  }

  if (eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx) == EGL_FALSE)
  {
    std::cerr << "Skipping test: eglMakeCurrent failed\n";
    eglDestroyContext(eglDpy, eglCtx);
    eglDestroySurface(eglDpy, eglSurf);
    eglTerminate(eglDpy);
    return EXIT_FAILURE;
  }

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

  std::cout << "eglGetProcAddress: " << (void*)eglGetProcAddress << '\n';
  std::cout << "egl(): " << (void*)f3d::context::egl()("eglGetProcAddress") << '\n';
  std::cout << "glGetString: " << (void*)eglGetProcAddress("glGetString") << '\n';
  std::cout << "glGetString 2: " << (void*)f3d::context::egl()("glGetString") << '\n';

  f3d::engine eng = f3d::engine::createExternalEGL();
  eng.getWindow().setSize(size[0], size[1]);
  eng.getScene().add(std::string(argv[1]) + "/data/cow.vtp");

  test("render with external EGL window",
    TestSDKHelpers::RenderTest(
      eng.getWindow(), std::string(argv[1]) + "baselines/", argv[2], "TestSDKExternalWindowEGL"));

  // terminate EGL when finished
  eglDestroyContext(eglDpy, eglCtx);
  eglDestroySurface(eglDpy, eglSurf);
  eglTerminate(eglDpy);

  return test.result();
}
