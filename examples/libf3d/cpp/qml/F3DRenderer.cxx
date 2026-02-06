#include "F3DRenderer.h"
#include "F3DView.h"
#include <QObject>
#include <QOpenGLContext>

#include <f3d/engine.h>
#include <f3d/options.h>
#include <f3d/scene.h>
#include <f3d/window.h>

//----------------------------------------------------------------------------
F3DRenderer::F3DRenderer()
{
  this->initialize();
}

//----------------------------------------------------------------------------
F3DRenderer::~F3DRenderer() = default;

//----------------------------------------------------------------------------
void F3DRenderer::initialize()
{
  f3d::engine::autoloadPlugins();

  f3d::context::function loadFunc = [](const char* name)
  { return QOpenGLContext::currentContext()->getProcAddress(name); };

  this->_engine = std::make_unique<f3d::engine>(f3d::engine::createExternal(loadFunc));

  if (!this->_engine)
  {
    qWarning() << "Failed to create F3D engine!";
    return;
  }

  qDebug() << "F3D Info: ";
  auto li = this->_engine->getLibInfo();
  qDebug() << "Version   : " << QString::fromStdString(li.VersionFull);
  qDebug() << "Build Date: " << QString::fromStdString(li.BuildDate);
  qDebug() << "Build Sys : " << QString::fromStdString(li.BuildSystem);
  qDebug() << "Compiler  : " << QString::fromStdString(li.Compiler);
  qDebug() << "VTK Ver.  : " << QString::fromStdString(li.VTKVersion);
  qDebug() << "License   : " << QString::fromStdString(li.License);
  qDebug() << "Modules:";
  for (auto& m : li.Modules)
  {
    qDebug() << " - " << QString::fromStdString(m.first) << ": " << m.second;
  }

  this->_interactor = &this->_engine->getInteractor();
  this->_frameTimer.start();
}

//----------------------------------------------------------------------------
void F3DRenderer::enqueue(const Event& ev)
{
  std::scoped_lock lock(this->_mutex);
  this->_events.push_back(ev);
  this->update();
}

//----------------------------------------------------------------------------
void F3DRenderer::queueMousePress(
  QPointF position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
  Event event{ EventType::MousePress, position.x(), position.y(), button, modifiers };
  this->enqueue(event);
}

//----------------------------------------------------------------------------
void F3DRenderer::queueMouseMove(
  QPointF position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
  Event event{ EventType::MouseMove, position.x(), position.y(), button, modifiers };
  this->enqueue(event);
}

//----------------------------------------------------------------------------
void F3DRenderer::queueMouseRelease(
  QPointF position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers)
{
  Event event{ EventType::MouseRelease, position.x(), position.y(), button, modifiers };
  this->enqueue(event);
}

//----------------------------------------------------------------------------
void F3DRenderer::queueWheel(QPoint angleDelta, Qt::KeyboardModifiers modifiers)
{
  Event event{ EventType::Wheel, 0, 0, Qt::MouseButton::NoButton, modifiers, angleDelta.x(),
    angleDelta.y() };
  this->enqueue(event);
}

//----------------------------------------------------------------------------
void F3DRenderer::queueKeyPress(int key, const QString& text, Qt::KeyboardModifiers modifiers)
{
  Event event{ EventType::KeyPress, 0, 0, Qt::MouseButton::NoButton, modifiers, 0, 0, key, text };
  this->enqueue(event);
}

//----------------------------------------------------------------------------
void F3DRenderer::queueKeyRelease(int key, Qt::KeyboardModifiers modifiers)
{
  Event event{ EventType::KeyRelease, 0, 0, Qt::MouseButton::NoButton, modifiers, 0, 0, key };
  this->enqueue(event);
}

//----------------------------------------------------------------------------
void F3DRenderer::updateModifiers(Qt::KeyboardModifiers mods)
{
  if (this->_interactor == nullptr)
  {
    return;
  }

  auto& inter = *this->_interactor;
  using InputMod = f3d::interactor::InputModifier;

  if ((mods & Qt::ControlModifier) && (mods & Qt::ShiftModifier))
  {
    inter.triggerModUpdate(InputMod::CTRL_SHIFT);
  }
  else if (mods & Qt::ControlModifier)
  {
    inter.triggerModUpdate(InputMod::CTRL);
  }
  else if (mods & Qt::ShiftModifier)
  {
    inter.triggerModUpdate(InputMod::SHIFT);
  }
  else
  {
    inter.triggerModUpdate(InputMod::NONE);
  }
}

int F3DRenderer::mapMouseButton(Qt::MouseButton button) const
{
  using MouseButton = f3d::interactor::MouseButton;

  if (button & Qt::LeftButton)
  {
    return static_cast<int>(MouseButton::LEFT);
  }

  if (button & Qt::RightButton)
  {
    return static_cast<int>(MouseButton::RIGHT);
  }

  return static_cast<int>(MouseButton::MIDDLE);
}

//----------------------------------------------------------------------------
std::string F3DRenderer::keySymFromKeyAndText(int key, const QString& text) const
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
    case Qt::Key_Shift:
      return "Shift_L";
    case Qt::Key_Control:
      return "Control_L";
    case Qt::Key_Alt:
      return "Alt_L";
    case Qt::Key_Meta:
      return "Super_L";
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
void F3DRenderer::handleEvent(const Event& ev)
{
  auto& inter = *this->_interactor;

  switch (ev.type)
  {
    case EventType::MousePress:
    {
      auto mouseButtonEnum =
        static_cast<f3d::interactor::MouseButton>(this->mapMouseButton(ev.button));
      inter.triggerMousePosition(ev.x, ev.y);
      this->updateModifiers(ev.modifiers);
      inter.triggerMouseButton(f3d::interactor::InputAction::PRESS, mouseButtonEnum);
      break;
    }
    case EventType::MouseMove:
    {
      inter.triggerMousePosition(ev.x, ev.y);
      this->updateModifiers(ev.modifiers);
      break;
    }
    case EventType::MouseRelease:
    {
      auto mouseButtonEnum =
        static_cast<f3d::interactor::MouseButton>(this->mapMouseButton(ev.button));
      inter.triggerMousePosition(ev.x, ev.y);
      this->updateModifiers(ev.modifiers);
      inter.triggerMouseButton(f3d::interactor::InputAction::RELEASE, mouseButtonEnum);
      break;
    }
    case EventType::Wheel:
    {
      this->updateModifiers(ev.modifiers);
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
      this->updateModifiers(ev.modifiers);
      std::string keySym = this->keySymFromKeyAndText(ev.key, ev.text);
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
      this->updateModifiers(ev.modifiers);
      std::string keySym = this->keySymFromKeyAndText(ev.key, {});
      if (!keySym.empty())
      {
        inter.triggerKeyboardKey(f3d::interactor::InputAction::RELEASE, keySym);
      }
      break;
    }
  }
}

//----------------------------------------------------------------------------
void F3DRenderer::updateSize(const QSizeF& size)
{
  auto& window = this->_engine->getWindow();
  window.setSize(static_cast<int>(size.width()), static_cast<int>(size.height()));
}

//----------------------------------------------------------------------------
void F3DRenderer::render()
{
  if (!this->_engine || !this->_interactor)
  {
    return;
  }

  std::deque<Event> localQueue;
  {
    std::scoped_lock lock(this->_mutex);
    localQueue.swap(this->_events);
  }

  for (const Event& event : localQueue)
  {
    this->handleEvent(event);
  }

  qint64 nSec = this->_frameTimer.nsecsElapsed();
  double dt = static_cast<double>(nSec) / 1e9;
  this->_interactor->triggerEventLoop(dt);
  this->_frameTimer.restart();

  this->_engine->getWindow().render();

  this->update();
}

//----------------------------------------------------------------------------
void F3DRenderer::synchronize(QQuickFramebufferObject* item)
{
  if (!this->_engine)
  {
    return;
  }

  auto* view = qobject_cast<F3DView*>(item);
  if (!view)
  {
    return;
  }

  const QSizeF size = view->size();
  if (size.isValid() && size != this->_lastSize)
  {
    this->updateSize(size);
    this->_lastSize = size;
  }

  QVariantMap& pendingOptions = view->pendingOptions();
  if (pendingOptions.size())
  {
    QMapIterator<QString, QVariant> it(pendingOptions);
    auto& options = this->_engine->getOptions();
    while (it.hasNext())
    {
      it.next();
      switch (it.value().typeId())
      {
        case QMetaType::Bool:
          options.set(it.key().toStdString(), it.value().toBool());
          break;
        case QMetaType::Int:
          options.set(it.key().toStdString(), it.value().toInt());
          break;
        case QMetaType::Double:
          options.set(it.key().toStdString(), it.value().toDouble());
          break;
        case QMetaType::QString:
          options.set(it.key().toStdString(), it.value().toString().toStdString());
          break;
        default:
          qWarning() << "Unsupported f3d option type for key " << it.key();
      }
    }
    pendingOptions.clear();
  }

  const QString modelPath = view->modelPath();
  if (!modelPath.isEmpty() && modelPath != this->_lastModelPath)
  {
    try
    {
      this->_engine->getScene().add(modelPath.toStdString());
      this->_lastModelPath = modelPath;
    }
    catch (const std::exception& ex)
    {
      qWarning() << "Failed to load model:" << modelPath << ex.what();
    }
  }
}
