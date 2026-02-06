#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <scene.h>
#include <window.h>

#include <GLFW/glfw3.h>

#include <iostream>

int TestSDKExternalWindowGLFW([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  // setup glfw window
  if (!glfwInit())
  {
    std::cerr << "Can't initialize GLFW.\n";
    return EXIT_FAILURE;
  }

  glfwSetErrorCallback([](int error, const char* desc) {
    std::cerr << "GLFW error " << error << ": " << desc << "\n";
  });
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(300, 300, "F3D GLFW External Window", nullptr, nullptr);

  if (!window)
  {
    std::cerr << "Can't create GLFW window.\n";
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  PseudoUnitTest test;

  f3d::engine eng = f3d::engine::createExternal(glfwGetProcAddress);
  eng.getWindow().setSize(300, 300);

  // key callback
  glfwSetKeyCallback(window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
      glfwSetWindowShouldClose(win, 1);
    }
  });

  eng.getScene().add(std::string(argv[1]) + "/data/cow.vtp");

  while (!glfwWindowShouldClose(window) && glfwGetTime() < 1.0)
  {
    eng.getWindow().render();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Ideally, we should not test the content of the window, but the GLFW framebuffer itself
  // There is currently no API in GLFW that allows to do that unfortunately
  test("render test with external GLFW window",
    TestSDKHelpers::RenderTest(
      eng.getWindow(), std::string(argv[1]) + "baselines/", argv[2], "TestSDKExternalWindowGLFW"));

  glfwTerminate();

  return test.result();
}
