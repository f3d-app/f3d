#include "F3DRenderer.h"
#include "F3DView.h"

#include <QOpenGLContext>

#include <f3d/engine.h>
#include <f3d/options.h>
#include <f3d/scene.h>
#include <f3d/window.h>

//----------------------------------------------------------------------------
F3DRenderer::F3DRenderer(F3DView* item)
  : View(item)
{
  f3d::engine::autoloadPlugins();

  f3d::context::function loadFunc = [](const char* name)
  { return QOpenGLContext::currentContext()->getProcAddress(name); };
  this->Engine = std::make_unique<f3d::engine>(f3d::engine::createExternal(loadFunc));

  if (!this->Engine)
  {
    return;
  }

  if (!this->View->GetModelPath().isEmpty())
  {
    this->Engine->getScene().add(this->View->GetModelPath().toStdString());
  }

  auto& window = this->Engine->getWindow();
  window.setSize(static_cast<int>(this->View->width()), static_cast<int>(this->View->height()));

  auto& options = this->Engine->getOptions();

  options.set("render.grid.enable", true);
  options.set("render.show_edges", false);
  options.set("ui.axis", true);
  options.set("ui.fps", true);
  options.set("ui.filename", true);

  options.set("render.background.color", f3d::color_t(0.15, 0.15, 0.15));

  this->Interactor = &this->Engine->getInteractor();

  this->FrameTimer.start();
  this->FirstFrame = true;
}

//----------------------------------------------------------------------------
F3DRenderer::~F3DRenderer() = default;

//----------------------------------------------------------------------------
void F3DRenderer::Enqueue(const Event& ev)
{
  std::lock_guard lock(this->Mutex);
  this->Events.push_back(ev);
}

//----------------------------------------------------------------------------
void F3DRenderer::QueueMousePress(float x, float y, int button, int modifiers)
{
  Event event;
  event.type = EventType::MousePress;
  event.x = x;
  event.y = y;
  event.button = button;
  event.modifiers = modifiers;

  this->Enqueue(event);
  this->update();
}

//----------------------------------------------------------------------------
void F3DRenderer::QueueMouseMove(float x, float y, int buttons, int modifiers)
{
  Event event;
  event.type = EventType::MouseMove;
  event.x = x;
  event.y = y;
  event.buttons = buttons;
  event.modifiers = modifiers;

  this->Enqueue(event);
  this->update();
}

//----------------------------------------------------------------------------
void F3DRenderer::QueueMouseRelease(float x, float y, int button, int modifiers)
{
  Event event;
  event.type = EventType::MouseRelease;
  event.x = x;
  event.y = y;
  event.button = button;
  event.modifiers = modifiers;

  this->Enqueue(event);
  this->update();
}

//----------------------------------------------------------------------------
void F3DRenderer::QueueWheel(int dx, int dy, int modifiers)
{
  Event event;
  event.type = EventType::Wheel;
  event.dx = dx;
  event.dy = dy;
  event.modifiers = modifiers;

  this->Enqueue(event);
  this->update();
}

//----------------------------------------------------------------------------
void F3DRenderer::QueueKeyPress(int key, const QString& text, int modifiers)
{
  Event event;
  event.type = EventType::KeyPress;
  event.key = key;
  event.text = text;
  event.modifiers = modifiers;

  this->Enqueue(event);
  this->update();
}

//----------------------------------------------------------------------------
void F3DRenderer::QueueKeyRelease(int key, int modifiers)
{
  Event event;
  event.type = EventType::KeyRelease;
  event.key = key;
  event.modifiers = modifiers;

  this->Enqueue(event);
  this->update();
}

//----------------------------------------------------------------------------
void F3DRenderer::UpdateModifiers(int mods)
{
  if (this->Interactor == nullptr)
  {
    return;
  }

  auto& inter = *this->Interactor;
  const auto qMods = Qt::KeyboardModifiers(mods);
  using InputMod = f3d::interactor::InputModifier;

  if ((qMods & Qt::ControlModifier) && (qMods & Qt::ShiftModifier))
  {
    inter.triggerModUpdate(InputMod::CTRL_SHIFT);
  }
  else if (qMods & Qt::ControlModifier)
  {
    inter.triggerModUpdate(InputMod::CTRL);
  }
  else if (qMods & Qt::ShiftModifier)
  {
    inter.triggerModUpdate(InputMod::SHIFT);
  }
  else
  {
    inter.triggerModUpdate(InputMod::NONE);
  }
}

//----------------------------------------------------------------------------
int F3DRenderer::MapMouseButton(int button) const
{
  auto qButton = static_cast<Qt::MouseButton>(button);
  using MouseButton = f3d::interactor::MouseButton;

  if (qButton == Qt::LeftButton)
  {
    return static_cast<int>(MouseButton::LEFT);
  }

  if (qButton == Qt::RightButton)
  {
    return static_cast<int>(MouseButton::RIGHT);
  }

  return static_cast<int>(MouseButton::MIDDLE);
}

//----------------------------------------------------------------------------
std::string F3DRenderer::KeySymFromKeyAndText(int key, const QString& text) const
{
  if (key >= Qt::Key_A && key <= Qt::Key_Z)
  {
    return std::string(1, static_cast<char>(std::toupper(key)));
  }

  switch (key)
  {
    case Qt::Key_Backspace:
      return "BackSpace";
    case Qt::Key_Escape:
      return "Escape";
    case Qt::Key_Return:
    case Qt::Key_Enter:
      return "Return";
    case Qt::Key_Space:
      return "Space";
    case Qt::Key_Tab:
      return "Tab";
    default:
      break;
  }

  if (!text.isEmpty())
  {
    return text.toStdString();
  }

  return {};
}

//----------------------------------------------------------------------------
void F3DRenderer::HandleEvent(const Event& ev)
{
  auto& inter = *this->Interactor;

  switch (ev.type)
  {
    case EventType::MousePress:
    {
      this->UpdateModifiers(ev.modifiers);
      auto mouseButtonEnum =
        static_cast<f3d::interactor::MouseButton>(this->MapMouseButton(ev.button));
      double yFlipped = this->View->height() - ev.y;
      inter.triggerMousePosition(ev.x, yFlipped);
      inter.triggerMouseButton(f3d::interactor::InputAction::PRESS, mouseButtonEnum);
      break;
    }
    case EventType::MouseMove:
    {
      this->UpdateModifiers(ev.modifiers);
      double yFlipped = this->View->height() - ev.y;
      inter.triggerMousePosition(ev.x, yFlipped);
      break;
    }
    case EventType::MouseRelease:
    {
      this->UpdateModifiers(ev.modifiers);
      auto mouseButtonEnum =
        static_cast<f3d::interactor::MouseButton>(this->MapMouseButton(ev.button));
      double yFlipped = this->View->height() - ev.y;
      inter.triggerMouseButton(f3d::interactor::InputAction::RELEASE, mouseButtonEnum);
      inter.triggerMousePosition(ev.x, yFlipped);
      break;
    }
    case EventType::Wheel:
    {
      this->UpdateModifiers(ev.modifiers);
      using WheelDirection = f3d::interactor::WheelDirection;

      WheelDirection direction;
      if (std::abs(ev.dy) >= std::abs(ev.dx))
      {
        direction = (ev.dy > 0) ? WheelDirection::FORWARD : WheelDirection::BACKWARD;
      }
      else
      {
        direction = (ev.dx > 0) ? WheelDirection::RIGHT : WheelDirection::LEFT;
      }

      inter.triggerMouseWheel(direction);
      break;
    }
    case EventType::KeyPress:
    {
      this->UpdateModifiers(ev.modifiers);
      std::string keySym = this->KeySymFromKeyAndText(ev.key, ev.text);
      if (!keySym.empty())
      {
        inter.triggerKeyboardKey(f3d::interactor::InputAction::PRESS, keySym);
      }
      if (!ev.text.isEmpty())
      {
        for (QChar ch : ev.text)
        {
          inter.triggerTextCharacter(ch.unicode());
        }
      }
      break;
    }
    case EventType::KeyRelease:
    {
      this->UpdateModifiers(ev.modifiers);
      std::string keySym = this->KeySymFromKeyAndText(ev.key, {});
      if (!keySym.empty())
      {
        inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, keySym);
      }
      break;
    }
  }
}

//----------------------------------------------------------------------------
void F3DRenderer::render()
{
  if (!this->Engine || !this->Interactor)
  {
    return;
  }

  auto& window = this->Engine->getWindow();
  window.setSize(static_cast<int>(this->View->width()), static_cast<int>(this->View->height()));

  std::deque<Event> localQueue;
  {
    std::lock_guard lock(this->Mutex);
    localQueue.swap(this->Events);
  }

  for (const Event& event : localQueue)
  {
    this->HandleEvent(event);
  }

  double dt = 1.0 / 30.0;

  if (this->FirstFrame)
  {
    this->FrameTimer.restart();
    this->FirstFrame = false;
  }
  else
  {
    qint64 nSec = this->FrameTimer.nsecsElapsed();
    this->FrameTimer.restart();
    dt = static_cast<double>(nSec) / 1e9;
  }
  this->Interactor->triggerEventLoop(dt);

  window.render();

  this->update();
}
