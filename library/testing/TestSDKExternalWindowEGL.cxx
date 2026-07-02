#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include "engine.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

int TestSDKExternalWindowEGL([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
    reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(eglGetProcAddress("eglQueryDevicesEXT"));
  PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
    reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
      eglGetProcAddress("eglGetPlatformDisplayEXT"));
  PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT =
    reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(eglGetProcAddress("eglQueryDeviceStringEXT"));

  EGLDeviceEXT devices[16];
  EGLint numDev = 0;

  if (eglQueryDevicesEXT)
  {
    eglQueryDevicesEXT(16, devices, &numDev);

    if (eglQueryDeviceStringEXT)
    {
      for (int i = 0; i < numDev; ++i)
      {
        const char* deviceExts = eglQueryDeviceStringEXT(devices[i], EGL_EXTENSIONS);
        std::cout << "EGL device " << i << " extensions: " << (deviceExts ? deviceExts : "(none)")
                  << '\n';
      }
    }

    if (numDev > 0 && eglGetPlatformDisplayEXT)
    {
      eglDpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[0], nullptr);
    }
  }

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

  test("render with external EGL window",
    TestSDKHelpers::RenderTest(
      eng.getWindow(), std::string(argv[1]) + "baselines/", argv[2], "TestSDKExternalWindowEGL"));

  // terminate EGL when finished
  eglDestroyContext(eglDpy, eglCtx);
  eglDestroySurface(eglDpy, eglSurf);
  eglTerminate(eglDpy);

  return test.result();
}
