#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Window.H>
#include <FL/gl.h>

#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/options.h>
#include <f3d/scene.h>
#include <f3d/window.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

class F3DGLWindow : public Fl_Gl_Window
{
public:
  explicit F3DGLWindow(int X, int Y, int W, int H, std::string filePath)
    : Fl_Gl_Window(X, Y, W, H, "libf3d + FLTK example")
    , FilePath(std::move(filePath))
  {
    this->mode(FL_DOUBLE | FL_RGB | FL_DEPTH | FL_ALPHA);
  }

  ~F3DGLWindow() override = default;

  void initialize()
  {
    if (this->Initialized)
    {
      return;
    }

    this->make_current();

    f3d::engine::autoloadPlugins();

    f3d::context::function loadFunc;
#if defined(_WIN32)
    loadFunc = f3d::context::wgl();
#elif defined(__APPLE__)
    loadFunc = f3d::context::cocoa();
#else
    const char* sessionType = std::getenv("XDG_SESSION_TYPE");

    if (sessionType && std::string(sessionType) == "wayland")
    {
      loadFunc = f3d::context::egl();
    }
    else
    {
      loadFunc = f3d::context::glx();
    }
#endif
    this->Engine = std::make_unique<f3d::engine>(f3d::engine::createExternal(loadFunc));

    if (!this->Engine)
    {
      std::cerr << "Failed to create f3d::engine" << '\n';
      return;
    }

    this->Interactor = &this->Engine->getInteractor();

    // Load the model
    this->Engine->getScene().add(this->FilePath);

    auto& win = this->Engine->getWindow();
    win.setSize(this->w(), this->h());

    auto& opt = this->Engine->getOptions();
    opt.set("render.grid.enable", true);
    opt.set("render.show_edges", false);
    opt.set("ui.axis", true);
    opt.set("ui.fps", true);
    opt.set("ui.filename", true);

    this->Initialized = true;

    Fl::add_timeout(1.0 / 30.0, &F3DGLWindow::TimerCallback, this);
  }

protected:
  void draw() override
  {
    if (!valid())
    {
      this->initialize();
      this->valid(1);
    }

    if (!this->Engine)
    {
      return;
    }

    // Process any pending mouse position updates before rendering
    if (this->HasPendingMouseMove && this->Interactor)
    {
      this->Interactor->triggerMousePosition(this->PendingMouseX, this->PendingMouseY);
      this->HasPendingMouseMove = false;
    }

    this->Engine->getWindow().render();
  }

  void resize(int x, int y, int w, int h) override
  {
    Fl_Gl_Window::resize(x, y, w, h);

    if (!this->Engine)
    {
      return;
    }

    this->Engine->getWindow().setSize(std::max(1, w), std::max(1, h));
  }

  void UpdateModifiers(int state)
  {
    if (!this->Interactor)
    {
      return;
    }

    using InputModifier = f3d::interactor::InputModifier;

    const bool ctrl = (state & FL_CTRL) != 0;
    const bool shift = (state & FL_SHIFT) != 0;

    if (ctrl && shift)
    {
      this->Interactor->triggerModUpdate(InputModifier::CTRL_SHIFT);
    }
    else if (ctrl)
    {
      this->Interactor->triggerModUpdate(InputModifier::CTRL);
    }
    else if (shift)
    {
      this->Interactor->triggerModUpdate(InputModifier::SHIFT);
    }
    else
    {
      this->Interactor->triggerModUpdate(InputModifier::NONE);
    }
  }

  std::optional<f3d::interactor::MouseButton> MapMouseButton(int button)
  {
    if (!this->Interactor)
    {
      return std::nullopt;
    }

    using MouseButton = f3d::interactor::MouseButton;

    switch (button)
    {
      case FL_LEFT_MOUSE:
        return MouseButton::LEFT;
      case FL_RIGHT_MOUSE:
        return MouseButton::RIGHT;
      case FL_MIDDLE_MOUSE:
        return MouseButton::MIDDLE;
      default:
        return std::nullopt;
    }
  }

  std::string KeySymFromEvent()
  {
    int key = Fl::event_key();

    if (std::isalpha(key))
    {
      return std::string{ static_cast<char>(std::toupper(key)) };
    }

    switch (key)
    {
      case FL_BackSpace:
        return "BackSpace";
      case FL_Escape:
        return "Escape";
      case FL_Enter:
      case FL_KP_Enter:
        return "Return";
      case ' ':
        return "Space";
      case FL_Tab:
        return "Tab";
      default:
        break;
    }

    int ch = Fl::event_text()[0];
    if (ch != 0)
    {
      return std::string{ 1, static_cast<char>(ch) };
    }

    return {};
  }

  int handle(int event) override
  {
    if (!this->Interactor)
    {
      return Fl_Gl_Window::handle(event);
    }

    switch (event)
    {
      case FL_PUSH:
      {
        this->UpdateModifiers(Fl::event_state());
        auto button = this->MapMouseButton(Fl::event_button());

        if (button.has_value())
        {
          const int x = Fl::event_x();
          const int y = Fl::event_y();
          this->Interactor->triggerMousePosition(x, y);
          this->Interactor->triggerMouseButton(f3d::interactor::InputAction::PRESS, button.value());
        }

        return 1;
      }

      case FL_DRAG:
      case FL_MOVE:
      {
        this->UpdateModifiers(Fl::event_state());
        const int x = Fl::event_x();
        const int y = Fl::event_y();

        // Store the position for the next draw cycle
        this->PendingMouseX = x;
        this->PendingMouseY = y;
        this->HasPendingMouseMove = true;

        return 1;
      }

      case FL_RELEASE:
      {
        this->UpdateModifiers(Fl::event_state());
        auto button = this->MapMouseButton(Fl::event_button());
        if (button.has_value())
        {
          const int x = Fl::event_x();
          const int y = Fl::event_y();
          this->Interactor->triggerMousePosition(x, y);
          this->Interactor->triggerMouseButton(
            f3d::interactor::InputAction::RELEASE, button.value());
        }

        return 1;
      }

      case FL_MOUSEWHEEL:
      {
        this->UpdateModifiers(Fl::event_state());

        using WheelDirection = f3d::interactor::WheelDirection;
        int dx = Fl::event_dx();
        int dy = Fl::event_dy();

        WheelDirection direction;
        if (std::abs(dy) >= std::abs(dx))
        {
          direction = (dy < 0) ? WheelDirection::FORWARD : WheelDirection::BACKWARD;
        }
        else
        {
          direction = (dx > 0) ? WheelDirection::RIGHT : WheelDirection::LEFT;
        }

        this->Interactor->triggerMouseWheel(direction);
        return 1;
      }

      case FL_KEYDOWN:
      case FL_SHORTCUT:
      {
        this->UpdateModifiers(Fl::event_state());
        const std::string keySym = this->KeySymFromEvent();

        if (!keySym.empty())
        {
          this->Interactor->triggerKeyboardKey(
            f3d::interactor::InputAction::PRESS, std::string_view(keySym));
        }

        const char* txt = Fl::event_text();
        int len = Fl::event_length();
        for (int i = 0; i < len; ++i)
        {
          char16_t ch = static_cast<unsigned char>(txt[i]);
          this->Interactor->triggerTextCharacter(ch);
        }
        return 1;
      }

      case FL_KEYUP:
      {
        this->UpdateModifiers(Fl::event_state());
        const std::string keySym = this->KeySymFromEvent();

        if (!keySym.empty())
        {
          this->Interactor->triggerKeyboardKey(
            f3d::interactor::InputAction::RELEASE, std::string_view(keySym));
        }
        return 1;
      }

      default:
        break;
    }

    return Fl_Gl_Window::handle(event);
  }

private:
  static void TimerCallback(void* userdata)
  {
    auto* self = static_cast<F3DGLWindow*>(userdata);
    if (self)
    {
      self->onTick();
      Fl::repeat_timeout(1.0 / 30.0, &F3DGLWindow::TimerCallback, userdata);
    }
  }

  void onTick()
  {
    if (!this->Engine || !this->Interactor)
    {
      return;
    }

    this->make_current();
    this->Interactor->triggerEventLoop(1.0 / 30.0);
    this->redraw();
  }

  std::string FilePath;
  std::unique_ptr<f3d::engine> Engine;
  f3d::interactor* Interactor = nullptr;
  bool Initialized = false;

  int PendingMouseX = 0;
  int PendingMouseY = 0;
  bool HasPendingMouseMove = false;
};

int main(int argc, char** argv)
{
  if (argc < 2 || argc > 3)
  {
    std::cerr << "Usage: " << argv[0] << " <file> [timeout]\n";
    return 1;
  }

  // Required model file
  std::string filePath = argv[1];

  if (!std::filesystem::exists(filePath))
  {
    std::cerr << "File not found: " << filePath << '\n';
    return 1;
  }

  int timeout = 0;
  if (argc == 3)
  {
    try
    {
      timeout = std::stoi(argv[2]);
      if (timeout < 1)
      {
        std::cerr << "Timeout must be a positive integer\n";
        return 1;
      }
    }
    catch (...)
    {
      std::cerr << "Invalid timeout value\n";
      return 1;
    }
  }

  Fl::visual(FL_RGB | FL_DOUBLE | FL_DEPTH);

  Fl_Window win(800, 600, "libf3d + FLTK example");
  F3DGLWindow glwin(0, 0, 800, 600, filePath);
  win.resizable(&glwin);
  win.end();
  win.show();

  glwin.take_focus();

  // For testing purposes only, shutdown the example after `timeout` seconds
  if (timeout > 0)
  {
    Fl::add_timeout(
      timeout,
      [](void* data)
      {
        auto* w = static_cast<Fl_Window*>(data);
        if (w)
        {
          w->hide();
        }
      },
      &win);
  }

  return Fl::run();
}
