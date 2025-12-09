#include <f3d/engine.h>
#include <f3d/scene.h>
#include <f3d/window.h>

#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>

f3d::interactor::MouseButton translateButton(int button)
{
  switch (button)
  {
    case GLFW_MOUSE_BUTTON_RIGHT:
      return f3d::interactor::MouseButton::RIGHT;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      return f3d::interactor::MouseButton::MIDDLE;
    case GLFW_MOUSE_BUTTON_LEFT:
    default:
      return f3d::interactor::MouseButton::LEFT;
  }
}

std::string translateKey(int key)
{
  switch (key)
  {
    case GLFW_KEY_SPACE:
      return "Space";
    case GLFW_KEY_APOSTROPHE:
      return "Apostrophe";
    case GLFW_KEY_COMMA:
      return "Comma";
    case GLFW_KEY_MINUS:
      return "Minus";
    case GLFW_KEY_PERIOD:
      return "Period";
    case GLFW_KEY_SLASH:
      return "Slash";
    case GLFW_KEY_0:
      return "0";
    case GLFW_KEY_1:
      return "1";
    case GLFW_KEY_2:
      return "2";
    case GLFW_KEY_3:
      return "3";
    case GLFW_KEY_4:
      return "4";
    case GLFW_KEY_5:
      return "5";
    case GLFW_KEY_6:
      return "6";
    case GLFW_KEY_7:
      return "7";
    case GLFW_KEY_8:
      return "8";
    case GLFW_KEY_9:
      return "9";
    case GLFW_KEY_SEMICOLON:
      return "Semicolon";
    case GLFW_KEY_EQUAL:
      return "Equal";
    case GLFW_KEY_A:
      return "A";
    case GLFW_KEY_B:
      return "B";
    case GLFW_KEY_C:
      return "C";
    case GLFW_KEY_D:
      return "D";
    case GLFW_KEY_E:
      return "E";
    case GLFW_KEY_F:
      return "F";
    case GLFW_KEY_G:
      return "G";
    case GLFW_KEY_H:
      return "H";
    case GLFW_KEY_I:
      return "I";
    case GLFW_KEY_J:
      return "J";
    case GLFW_KEY_K:
      return "K";
    case GLFW_KEY_L:
      return "L";
    case GLFW_KEY_M:
      return "M";
    case GLFW_KEY_N:
      return "N";
    case GLFW_KEY_O:
      return "O";
    case GLFW_KEY_P:
      return "P";
    case GLFW_KEY_Q:
      return "Q";
    case GLFW_KEY_R:
      return "R";
    case GLFW_KEY_S:
      return "S";
    case GLFW_KEY_T:
      return "T";
    case GLFW_KEY_U:
      return "U";
    case GLFW_KEY_V:
      return "V";
    case GLFW_KEY_W:
      return "W";
    case GLFW_KEY_X:
      return "X";
    case GLFW_KEY_Y:
      return "Y";
    case GLFW_KEY_Z:
      return "Z";
    case GLFW_KEY_LEFT_BRACKET:
      return "LeftBracket";
    case GLFW_KEY_BACKSLASH:
      return "Backslash";
    case GLFW_KEY_RIGHT_BRACKET:
      return "RightBracket";
    case GLFW_KEY_GRAVE_ACCENT:
      return "GraveAccent";
    case GLFW_KEY_ESCAPE:
      return "Escape";
    case GLFW_KEY_ENTER:
      return "Return";
    case GLFW_KEY_TAB:
      return "Tab";
    case GLFW_KEY_BACKSPACE:
      return "BackSpace";
    case GLFW_KEY_INSERT:
      return "Insert";
    case GLFW_KEY_DELETE:
      return "Delete";
    case GLFW_KEY_RIGHT:
      return "Right";
    case GLFW_KEY_LEFT:
      return "Left";
    case GLFW_KEY_DOWN:
      return "Down";
    case GLFW_KEY_UP:
      return "Up";
    case GLFW_KEY_PAGE_UP:
      return "Prior";
    case GLFW_KEY_PAGE_DOWN:
      return "Next";
    case GLFW_KEY_HOME:
      return "Home";
    case GLFW_KEY_END:
      return "End";
    case GLFW_KEY_CAPS_LOCK:
      return "CapsLock";
    case GLFW_KEY_SCROLL_LOCK:
      return "ScrollLock";
    case GLFW_KEY_NUM_LOCK:
      return "NumLock";
    case GLFW_KEY_PRINT_SCREEN:
      return "PrintScreen";
    case GLFW_KEY_PAUSE:
      return "Pause";
    case GLFW_KEY_F1:
      return "F1";
    case GLFW_KEY_F2:
      return "F2";
    case GLFW_KEY_F3:
      return "F3";
    case GLFW_KEY_F4:
      return "F4";
    case GLFW_KEY_F5:
      return "F5";
    case GLFW_KEY_F6:
      return "F6";
    case GLFW_KEY_F7:
      return "F7";
    case GLFW_KEY_F8:
      return "F8";
    case GLFW_KEY_F9:
      return "F9";
    case GLFW_KEY_F10:
      return "F10";
    case GLFW_KEY_F11:
      return "F11";
    case GLFW_KEY_F12:
      return "F12";
    case GLFW_KEY_F13:
      return "F13";
    case GLFW_KEY_F14:
      return "F14";
    case GLFW_KEY_F15:
      return "F15";
    case GLFW_KEY_F16:
      return "F16";
    case GLFW_KEY_F17:
      return "F17";
    case GLFW_KEY_F18:
      return "F18";
    case GLFW_KEY_F19:
      return "F19";
    case GLFW_KEY_F20:
      return "F20";
    case GLFW_KEY_F21:
      return "F21";
    case GLFW_KEY_F22:
      return "F22";
    case GLFW_KEY_F23:
      return "F23";
    case GLFW_KEY_F24:
      return "F24";
    case GLFW_KEY_F25:
      return "F25";
    case GLFW_KEY_KP_0:
      return "KP_0";
    case GLFW_KEY_KP_1:
      return "KP_1";
    case GLFW_KEY_KP_2:
      return "KP_2";
    case GLFW_KEY_KP_3:
      return "KP_3";
    case GLFW_KEY_KP_4:
      return "KP_4";
    case GLFW_KEY_KP_5:
      return "KP_5";
    case GLFW_KEY_KP_6:
      return "KP_6";
    case GLFW_KEY_KP_7:
      return "KP_7";
    case GLFW_KEY_KP_8:
      return "KP_8";
    case GLFW_KEY_KP_9:
      return "KP_9";
    case GLFW_KEY_KP_DECIMAL:
      return "KP_Decimal";
    case GLFW_KEY_KP_DIVIDE:
      return "KP_Divide";
    case GLFW_KEY_KP_MULTIPLY:
      return "KP_Multiply";
    case GLFW_KEY_KP_SUBTRACT:
      return "KP_Subtract";
    case GLFW_KEY_KP_ADD:
      return "KP_Add";
    case GLFW_KEY_KP_ENTER:
      return "KP_Enter";
    case GLFW_KEY_KP_EQUAL:
      return "KP_Equal";
    case GLFW_KEY_LEFT_SHIFT:
      return "LeftShift";
    case GLFW_KEY_LEFT_CONTROL:
      return "LeftControl";
    case GLFW_KEY_LEFT_ALT:
      return "LeftAlt";
    case GLFW_KEY_LEFT_SUPER:
      return "LeftSuper";
    case GLFW_KEY_RIGHT_SHIFT:
      return "RightShift";
    case GLFW_KEY_RIGHT_CONTROL:
      return "RightControl";
    case GLFW_KEY_RIGHT_ALT:
      return "RightAlt";
    case GLFW_KEY_RIGHT_SUPER:
      return "RightSuper";
    case GLFW_KEY_MENU:
      return "Menu";
    default:
      return ""; // Unknown key
  }
}

f3d::interactor::InputAction translateAction(int action)
{
  switch (action)
  {
    case GLFW_RELEASE:
      return f3d::interactor::InputAction::RELEASE;
    case GLFW_PRESS:
    default:
      return f3d::interactor::InputAction::PRESS;
  }
}

f3d::interactor::InputModifier translateMod(int mods)
{
  bool shift = mods & GLFW_MOD_SHIFT;
  bool ctrl = mods & GLFW_MOD_CONTROL;

  if (shift && ctrl)
  {
    return f3d::interactor::InputModifier::CTRL_SHIFT;
  }

  if (shift)
  {
    return f3d::interactor::InputModifier::SHIFT;
  }

  if (ctrl)
  {
    return f3d::interactor::InputModifier::CTRL;
  }

  return f3d::interactor::InputModifier::NONE;
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
    return EXIT_FAILURE;
  }

  int timeout = 0;
  if (argc > 2)
  {
    timeout = std::atoi(argv[2]);
  }
  auto startTime = std::chrono::steady_clock::now();

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

  int defaultSize[] = { 1920, 1080 };

  GLFWwindow* window =
    glfwCreateWindow(defaultSize[0], defaultSize[1], "F3D GLFW External Window", nullptr, nullptr);

  if (!window)
  {
    std::cerr << "Can't create GLFW window." << std::endl;
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  {
    f3d::engine::autoloadPlugins();

    f3d::engine eng = f3d::engine::createExternal(glfwGetProcAddress);
    eng.getWindow().setSize(defaultSize[0], defaultSize[1]);

    glfwSetWindowUserPointer(window, &eng);

    // resize callback
    glfwSetFramebufferSizeCallback(window,
      [](GLFWwindow* window, int width, int height)
      {
        f3d::engine* pe = static_cast<f3d::engine*>(glfwGetWindowUserPointer(window));
        pe->getWindow().setSize(width, height);
      });

    // key callback
    glfwSetKeyCallback(window,
      [](GLFWwindow* window, int key, int scancode, int action, int mods)
      {
        f3d::engine* pe = static_cast<f3d::engine*>(glfwGetWindowUserPointer(window));
        pe->getInteractor().triggerModUpdate(translateMod(mods));
        pe->getInteractor().triggerKeyboardKey(translateAction(action), translateKey(key));
      });

    glfwSetCharCallback(window,
      [](GLFWwindow* window, unsigned int codepoint)
      {
        f3d::engine* pe = static_cast<f3d::engine*>(glfwGetWindowUserPointer(window));
        pe->getInteractor().triggerTextCharacter(codepoint);
      });

    // mouse callback
    glfwSetMouseButtonCallback(window,
      [](GLFWwindow* window, int button, int action, int mods)
      {
        f3d::engine* pe = static_cast<f3d::engine*>(glfwGetWindowUserPointer(window));
        pe->getInteractor().triggerModUpdate(translateMod(mods));
        pe->getInteractor().triggerMouseButton(translateAction(action), translateButton(button));
      });

    glfwSetCursorPosCallback(window,
      [](GLFWwindow* window, double xpos, double ypos)
      {
        f3d::engine* pe = static_cast<f3d::engine*>(glfwGetWindowUserPointer(window));
        pe->getInteractor().triggerMousePosition(xpos, ypos);
        pe->getWindow().render();
        glfwSwapBuffers(window);
      });

    glfwSetScrollCallback(window,
      [](GLFWwindow* window, double xoffset, double yoffset)
      {
        if (yoffset != 0.0)
        {
          f3d::engine* pe = static_cast<f3d::engine*>(glfwGetWindowUserPointer(window));
          pe->getInteractor().triggerMouseWheel(yoffset > 0
              ? f3d::interactor::WheelDirection::FORWARD
              : f3d::interactor::WheelDirection::BACKWARD);
          glfwSwapBuffers(window);
        }
      });

    eng.getScene().add(argv[1]);

    while (!glfwWindowShouldClose(window))
    {
      // For testing purposes only, shutdown the example after `timeout` seconds
      if (timeout > 0)
      {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

        if (elapsed >= timeout)
        {
          glfwSetWindowShouldClose(window, GLFW_TRUE);
          break;
        }
      }

      eng.getWindow().render();
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
