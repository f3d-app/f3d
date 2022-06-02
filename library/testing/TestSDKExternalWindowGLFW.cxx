#include <engine.h>
#include <loader.h>
#include <options.h>
#include <window.h>

#include "TestSDKHelpers.h"

#include <GLFW/glfw3.h>

#include <iostream>

int TestSDKExternalWindowGLFW(int argc, char* argv[])
{
  // create engine and load file
  f3d::engine eng(f3d::engine::CREATE_WINDOW | f3d::engine::WINDOW_EXTERNAL);
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/cow.vtp");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);

  // setup glfw window
  if (!glfwInit())
  {
    std::cerr << "Can't initialize GLFW." << std::endl;
    return EXIT_FAILURE;
  }

  glfwSetErrorCallback([](int error, const char* desc)
    { std::cerr << "GLFW error " << error << ": " << desc << std::endl; });
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(300, 300, "F3D GLFW External Window", nullptr, nullptr);

  if (!window)
  {
    std::cerr << "Can't create GLFW window." << std::endl;
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, &eng);

  // resize callback
  glfwSetWindowSizeCallback(window,
    [](GLFWwindow* window, int width, int height)
    {
      // notify f3d that the resolution has changed
      f3d::engine* eng = static_cast<f3d::engine*>(glfwGetWindowUserPointer(window));
      eng->getOptions().set("resolution", { width, height });
      eng->getWindow().update();
    });

  // key callback
  glfwSetKeyCallback(window,
    [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
      if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      {
        glfwSetWindowShouldClose(window, 1);
      }
    });

  while (!glfwWindowShouldClose(window) && glfwGetTime() < 1.0)
  {
    eng.getWindow().render();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Ideally, we should not test the content of the window, but the GLFW framebuffer itself
  // There is currently no API in GLFW that allows to do that unfortunately
  if (!TestSDKHelpers::RenderTest(
        eng.getWindow(), std::string(argv[1]) + "baselines/", argv[2], "TestSDKExternalWindowGLFW"))
  {
    return EXIT_FAILURE;
  }

  glfwTerminate();

  return EXIT_SUCCESS;
}
