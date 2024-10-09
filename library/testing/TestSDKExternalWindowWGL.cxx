// Copyright 2008 Arne Reiners
#include "engine.h"

#include "TestSDKHelpers.h"

#include <windows.h>

#include <GL/gl.h>

std::unique_ptr<f3d::engine> engine;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_PAINT:
      engine->getWindow().render();
      ValidateRect(hwnd, nullptr);
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int TestSDKExternalWindowWGL(int argc, char* argv[])
{
  HINSTANCE hInstance = GetModuleHandle(NULL);

  // Define window class
  const char* className = "F3DWin32Class";

  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = className;

  // Register window class
  RegisterClass(&wc);

  // Create window
  constexpr int size[] = { 300, 300 };
  HWND hwnd = CreateWindowEx(0, className, "F3D Win32 Example", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
    CW_USEDEFAULT, size[0], size[1], nullptr, nullptr, hInstance, nullptr);

  if (hwnd == nullptr)
  {
    return 0;
  }

  ShowWindow(hwnd, SW_HIDE);

  // Initialize OpenGL
  HDC hDC = GetDC(hwnd);

  PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1 };
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pixelFormat = ChoosePixelFormat(hDC, &pfd);
  SetPixelFormat(hDC, pixelFormat, &pfd);

  // Create and enable the OpenGL rendering context (RC)
  HGLRC hGLRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, hGLRC);

  engine = std::make_unique<f3d::engine>(f3d::engine::createExternalWGL());
  engine->getWindow().setSize(size[0], size[1]);
  engine->getScene().add(std::string(argv[1]) + "/data/cow.vtp");

  if (!TestSDKHelpers::RenderTest(engine->getWindow(), std::string(argv[1]) + "baselines/", argv[2],
        "TestSDKExternalWindowWGL"))
  {
    return EXIT_FAILURE;
  }

  // Disable the RC and delete it
  wglMakeCurrent(nullptr, nullptr);
  wglDeleteContext(hGLRC);

  // Release the DC
  ReleaseDC(hwnd, hDC);

  return 0;
}
