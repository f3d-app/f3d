#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QMainWindow>
#include <QOpenGLContext>
#include <QOpenGLWidget>
#include <QTimer>
#include <QWheelEvent>

#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/options.h>
#include <f3d/scene.h>
#include <f3d/window.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <utility>

class F3DWidget : public QOpenGLWidget
{
public:
  explicit F3DWidget(QString filePath, QWidget* parent = nullptr)
    : QOpenGLWidget(parent)
    , FilePath(std::move(filePath))
  {
    // Allow this widget to receive key events
    this->setFocusPolicy(Qt::StrongFocus);
  }

protected:
  // ------------------------------------------------------------------
  // OpenGL / F3D setup
  // ------------------------------------------------------------------

  void initializeGL() override
  {
    f3d::engine::autoloadPlugins();

    f3d::context::function loadFunc = [this](const char* name)
    { return this->context()->getProcAddress(name); };
    this->Engine = std::make_unique<f3d::engine>(f3d::engine::createExternal(loadFunc));

    if (!this->Engine)
    {
      std::cerr << "Failed to create f3d::engine" << '\n';
      return;
    }

    this->Interactor = &this->Engine->getInteractor();

    this->Timer = new QTimer(this);
    this->Timer->setInterval(30);
    connect(this->Timer, &QTimer::timeout, this, &F3DWidget::onTick);
    this->Timer->start();

    this->Engine->getScene().add(this->FilePath.toStdString());

    auto& win = this->Engine->getWindow();
    win.setSize(this->width(), this->height());

    auto& opt = this->Engine->getOptions();
    opt.set("render.grid.enable", true);
    opt.set("render.show_edges", false);
    opt.set("ui.axis", true);
    opt.set("ui.fps", true);
    opt.set("ui.filename", true);
  }

  void resizeGL(int width, int height) override
  {
    if (!this->Engine)
    {
      return;
    }

    this->Engine->getWindow().setSize(std::max(1, width), std::max(1, height));
  }

  void paintGL() override
  {
    if (!this->Engine)
    {
      return;
    }

    this->Engine->getWindow().render();
  }

  // ------------------------------------------------------------------
  // Forwarding helpers
  // ------------------------------------------------------------------

  void UpdateModifiers(Qt::KeyboardModifiers mods)
  {
    if (!this->Interactor)
    {
      return;
    }

    using InputModifier = f3d::interactor::InputModifier;

    if ((mods & Qt::ControlModifier) && (mods & Qt::ShiftModifier))
    {
      this->Interactor->triggerModUpdate(InputModifier::CTRL_SHIFT);
    }
    else if (mods & Qt::ControlModifier)
    {
      this->Interactor->triggerModUpdate(InputModifier::CTRL);
    }
    else if (mods & Qt::ShiftModifier)
    {
      this->Interactor->triggerModUpdate(InputModifier::SHIFT);
    }
    else
    {
      this->Interactor->triggerModUpdate(InputModifier::NONE);
    }
  }

  std::optional<f3d::interactor::MouseButton> MapMouseButton(Qt::MouseButton button)
  {
    if (!this->Interactor)
    {
      return std::nullopt;
    }

    using MouseButton = f3d::interactor::MouseButton;

    switch (button)
    {
      case Qt::LeftButton:
        return MouseButton::LEFT;
      case Qt::RightButton:
        return MouseButton::RIGHT;
      case Qt::MiddleButton:
        return MouseButton::MIDDLE;
      default:
        return std::nullopt;
    }
  }

  std::string KeySymFromEvent(QKeyEvent* event)
  {
    int key = event->key();

    // A–Z -> "A"..."Z"
    if (key >= Qt::Key_A && key <= Qt::Key_Z)
    {
      char c = static_cast<char>(key);
      if (c >= 'a' && c <= 'z')
      {
        c = static_cast<char>(c - 'a' + 'A');
      }
      return std::string{ c };
    }

    // Special keys
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

    const QString text = event->text();
    if (!text.isEmpty())
    {
      return text.toStdString();
    }

    return {};
  }

  // ------------------------------------------------------------------
  // Mouse interaction forwarding
  // ------------------------------------------------------------------

  void mousePressEvent(QMouseEvent* event) override
  {
    if (!this->Interactor)
    {
      QOpenGLWidget::mousePressEvent(event);
      return;
    }

    this->makeCurrent();

    auto button = this->MapMouseButton(event->button());

    if (button.has_value())
    {
      const QPointF pos = event->position();
      this->Interactor->triggerMousePosition(pos.x(), pos.y());
      this->UpdateModifiers(event->modifiers());
      this->Interactor->triggerMouseButton(f3d::interactor::InputAction::PRESS, button.value());
      this->update();
    }

    this->doneCurrent();
  }

  void mouseMoveEvent(QMouseEvent* event) override
  {
    if (!this->Interactor)
    {
      QOpenGLWidget::mouseMoveEvent(event);
      return;
    }

    this->makeCurrent();

    const QPointF pos = event->position();
    this->Interactor->triggerMousePosition(pos.x(), pos.y());
    this->UpdateModifiers(event->modifiers());
    this->update();

    this->doneCurrent();
  }

  void mouseReleaseEvent(QMouseEvent* event) override
  {
    if (!this->Interactor)
    {
      QOpenGLWidget::mouseReleaseEvent(event);
      return;
    }

    this->makeCurrent();

    auto button = this->MapMouseButton(event->button());
    if (button.has_value())
    {
      const QPointF pos = event->position();
      this->Interactor->triggerMousePosition(pos.x(), pos.y());
      this->UpdateModifiers(event->modifiers());
      this->Interactor->triggerMouseButton(f3d::interactor::InputAction::RELEASE, button.value());
      this->update();
    }

    this->doneCurrent();
  }

  void wheelEvent(QWheelEvent* event) override
  {
    if (!this->Interactor)
    {
      QOpenGLWidget::wheelEvent(event);
      return;
    }

    this->makeCurrent();

    this->UpdateModifiers(event->modifiers());

    using WheelDirection = f3d::interactor::WheelDirection;
    const QPoint delta = event->angleDelta();
    WheelDirection direction;

    if (std::abs(delta.y()) >= std::abs(delta.x()))
    {
      direction = (delta.y() > 0) ? WheelDirection::FORWARD : WheelDirection::BACKWARD;
    }
    else
    {
      direction = (delta.x() > 0) ? WheelDirection::RIGHT : WheelDirection::LEFT;
    }

    this->Interactor->triggerMouseWheel(direction);
    this->update();

    this->doneCurrent();
  }

  // ------------------------------------------------------------------
  // Keyboard interaction forwarding
  // ------------------------------------------------------------------

  void keyPressEvent(QKeyEvent* event) override
  {
    if (!this->Interactor)
    {
      QOpenGLWidget::keyPressEvent(event);
      return;
    }

    this->makeCurrent();

    this->UpdateModifiers(event->modifiers());
    const std::string keySym = this->KeySymFromEvent(event);

    if (!keySym.empty())
    {
      this->Interactor->triggerKeyboardKey(
        f3d::interactor::InputAction::PRESS, std::string_view(keySym));
    }

    const QString text = event->text();
    if (!text.isEmpty())
    {
      const std::u16string u16 = text.toStdU16String();
      for (char16_t ch : u16)
      {
        this->Interactor->triggerTextCharacter(ch);
      }
    }

    this->update();
    this->doneCurrent();
  }

  void keyReleaseEvent(QKeyEvent* event) override
  {
    if (!this->Interactor)
    {
      QOpenGLWidget::keyReleaseEvent(event);
      return;
    }

    this->makeCurrent();

    this->UpdateModifiers(event->modifiers());
    const std::string keySym = this->KeySymFromEvent(event);

    if (!keySym.empty())
    {
      this->Interactor->triggerKeyboardKey(
        f3d::interactor::InputAction::RELEASE, std::string_view(keySym));
    }

    this->update();
    this->doneCurrent();
  }

private slots:
  void onTick()
  {
    if (!this->Engine || !this->Interactor)
    {
      return;
    }

    this->makeCurrent();
    this->Interactor->triggerEventLoop(1.0 / 30.0);
    this->update();
    this->doneCurrent();
  }

private:
  QString FilePath;
  std::unique_ptr<f3d::engine> Engine;
  f3d::interactor* Interactor = nullptr;
  QTimer* Timer = nullptr;
};

class MainWindow : public QMainWindow
{
public:
  explicit MainWindow(const QString& filePath, QWidget* parent = nullptr)
    : QMainWindow(parent)
  {
    this->setWindowTitle(QStringLiteral("libf3d + Qt6 example"));
    auto viewer = std::make_unique<F3DWidget>(filePath, this);
    this->setCentralWidget(viewer.get());
    viewer.release();
    this->resize(800, 600);
  }
};

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  QCommandLineParser parser;
  parser.setApplicationDescription("libf3d + Qt6 example");
  parser.addHelpOption();

  QCommandLineOption timeoutOpt(
    "timeout", "Optional timeout (in seconds) before closing the viewer.", "seconds");
  parser.addOption(timeoutOpt);

  parser.addPositionalArgument("file", "Model file to open.");

  parser.process(app);

  const QStringList positional = parser.positionalArguments();
  if (positional.isEmpty())
  {
    std::cerr << "Usage: " << argv[0] << " <file> [--timeout N]" << '\n';
    return 1;
  }

  const QString& filePath = positional.first();

  if (!std::filesystem::exists(filePath.toStdString()))
  {
    std::cerr << "File not found: " << filePath.toStdString() << '\n';
    return 1;
  }

  MainWindow win(filePath);
  win.show();

  // For testing purposes only, shutdown the example after `timeout` seconds
  if (parser.isSet(timeoutOpt))
  {
    bool ok = false;
    int timeoutSec = parser.value(timeoutOpt).toInt(&ok);
    if (ok && timeoutSec > 0)
    {
      QTimer::singleShot(timeoutSec * 1000, &app, &QCoreApplication::quit);
    }
  }

  return app.exec();
}
