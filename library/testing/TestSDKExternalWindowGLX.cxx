// Copyright 2008 Arne Reiners

#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include "engine.h"

#include <GL/gl.h>
#include <GL/glx.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

using glXCreateContextAttribsARBProc = GLXContext (*)(
  Display*, GLXFBConfig, GLXContext, Bool, const int*);

int TestSDKExternalWindowGLX([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  Display* display = XOpenDisplay(nullptr);

  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;

  const char* extensions = glXQueryExtensionsString(display, DefaultScreen(display));
  std::cout << extensions << "\n";

  static int visual_attribs[] = { GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DOUBLEBUFFER, true, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, None };

  std::cout << "Getting framebuffer config\n";
  int fbcount;
  GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
  if (!fbc)
  {
    std::cerr << "Failed to retrieve a framebuffer config\n";
    return EXIT_FAILURE;
  }

  std::cout << "Getting XVisualInfo\n";
  XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[0]);

  XSetWindowAttributes swa;
  std::cout << "Creating colormap\n";
  swa.colormap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask;

  std::cout << "Creating window\n";
  Window win = XCreateWindow(display, RootWindow(display, vi->screen), 0, 0, 100, 100, 0, vi->depth,
    InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
  if (!win)
  {
    std::cerr << "Failed to create window.\n";
    return EXIT_FAILURE;
  }

  std::cout << "Mapping window\n";
  XMapWindow(display, win);

  // Create an oldstyle context first, to get the correct function pointer for
  // glXCreateContextAttribsARB
  GLXContext ctx_old = glXCreateContext(display, vi, nullptr, GL_TRUE);
  glXCreateContextAttribsARB = reinterpret_cast<glXCreateContextAttribsARBProc>(
    glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB")));
  glXMakeCurrent(display, 0, nullptr);
  glXDestroyContext(display, ctx_old);

  if (glXCreateContextAttribsARB == nullptr)
  {
    std::cerr << "glXCreateContextAttribsARB entry point not found. Aborting.\n";
    return EXIT_FAILURE;
  }

  static int context_attribs[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, 3, GLX_CONTEXT_MINOR_VERSION_ARB,
    2, None };

  std::cout << "Creating context\n";
  GLXContext ctx = glXCreateContextAttribsARB(display, fbc[0], nullptr, true, context_attribs);
  if (!ctx)
  {
    std::cerr << "Failed to create GL3 context.\n";
    return EXIT_FAILURE;
  }

  std::cout << "Making context current\n";
  glXMakeCurrent(display, win, ctx);

  PseudoUnitTest test;

  f3d::engine eng = f3d::engine::createExternalGLX();
  eng.getWindow().setSize(300, 300);
  eng.getScene().add(std::string(argv[1]) + "/data/cow.vtp");

  test("render with external GLX window",
    TestSDKHelpers::RenderTest(
      eng.getWindow(), std::string(argv[1]) + "baselines/", argv[2], "TestSDKExternalWindowGLX"));

  ctx = glXGetCurrentContext();
  glXMakeCurrent(display, 0, nullptr);
  glXDestroyContext(display, ctx);

  return test.result();
}
