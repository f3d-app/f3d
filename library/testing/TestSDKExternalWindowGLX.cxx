// Copyright 2008 Arne Reiners
#include "engine.h"

#include "TestSDKHelpers.h"

#include <GL/gl.h>
#include <GL/glx.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
using glXCreateContextAttribsARBProc = GLXContext (*)(
  Display*, GLXFBConfig, GLXContext, Bool, const int*);

int TestSDKExternalWindowGLX(int argc, char* argv[])
{
  Display* display = XOpenDisplay(nullptr);

  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;

  const char* extensions = glXQueryExtensionsString(display, DefaultScreen(display));
  std::cout << extensions << std::endl;

  static int visual_attribs[] = { GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DOUBLEBUFFER, true, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, None };

  std::cout << "Getting framebuffer config" << std::endl;
  int fbcount;
  GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
  if (!fbc)
  {
    std::cerr << "Failed to retrieve a framebuffer config" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Getting XVisualInfo" << std::endl;
  XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[0]);

  XSetWindowAttributes swa;
  std::cout << "Creating colormap" << std::endl;
  swa.colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask;

  std::cout << "Creating window" << std::endl;
  Window win = XCreateWindow(display, RootWindow(display, vi->screen), 0, 0, 100, 100, 0, vi->depth,
    InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
  if (!win)
  {
    std::cerr << "Failed to create window." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Mapping window" << std::endl;
  XMapWindow(display, win);

  // Create an oldstyle context first, to get the correct function pointer for
  // glXCreateContextAttribsARB
  GLXContext ctx_old = glXCreateContext(display, vi, nullptr, GL_TRUE);
  glXCreateContextAttribsARB =
    (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
  glXMakeCurrent(display, 0, nullptr);
  glXDestroyContext(display, ctx_old);

  if (glXCreateContextAttribsARB == nullptr)
  {
    std::cerr << "glXCreateContextAttribsARB entry point not found. Aborting." << std::endl;
    return EXIT_FAILURE;
  }

  static int context_attribs[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, 3, GLX_CONTEXT_MINOR_VERSION_ARB,
    2, None };

  std::cout << "Creating context" << std::endl;
  GLXContext ctx = glXCreateContextAttribsARB(display, fbc[0], nullptr, true, context_attribs);
  if (!ctx)
  {
    std::cerr << "Failed to create GL3 context." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Making context current" << std::endl;
  glXMakeCurrent(display, win, ctx);

  f3d::engine eng = f3d::engine::createExternalGLX();
  eng.getWindow().setSize(300, 300);
  eng.getScene().add(std::string(argv[1]) + "/data/cow.vtp");

  if (!TestSDKHelpers::RenderTest(
        eng.getWindow(), std::string(argv[1]) + "baselines/", argv[2], "TestSDKExternalWindowGLX"))
  {
    return EXIT_FAILURE;
  }

  ctx = glXGetCurrentContext();
  glXMakeCurrent(display, 0, nullptr);
  glXDestroyContext(display, ctx);
  return EXIT_SUCCESS;
}
