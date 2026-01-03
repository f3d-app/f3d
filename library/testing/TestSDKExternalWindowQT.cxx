#include "TestSDKHelpers.h"

#include "engine.h"
#include "interactor.h"
#include "options.h"
#include "scene.h"
#include "window.h"

#include <QApplication>
#include <QCoreApplication>
#include <QMainWindow>
#include <QOpenGLContext>
#include <QOpenGLWidget>
#include <QTimer>
#include <QtGlobal>

class F3DWidget : public QOpenGLWidget
{
public:
  explicit F3DWidget(std::string filePath, std::string baselinePath, std::string outputPath,
    QWidget* parent = nullptr)
    : QOpenGLWidget(parent)
    , mFilePath(std::move(filePath))
    , mBaselinePath(std::move(baselinePath))
    , mOutputPath(std::move(outputPath))
  {
  }

  bool checkFrameBuffer()
  {
    // before closing, compare the content of the framebuffer with the baseline
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    QImage frameBuffer = grabFramebuffer().flipped().convertToFormat(QImage::Format_RGB888);
#else
    QImage frameBuffer = grabFramebuffer().mirrored().convertToFormat(QImage::Format_RGB888);
#endif

    f3d::image img(frameBuffer.width(), frameBuffer.height(), 3);
    img.setContent(frameBuffer.bits());

    if (!TestSDKHelpers::RenderTest(
          img, this->mBaselinePath, this->mOutputPath, "TestSDKExternalWindowQT"))
    {
      return false;
    }
    return true;
  }

protected:
  void initializeGL() override
  {
    f3d::engine::autoloadPlugins();

    f3d::context::function loadFunc = [this](const char* name) {
      return this->context()->getProcAddress(name);
    };
    this->mEngine = std::make_unique<f3d::engine>(f3d::engine::createExternal(loadFunc));

    if (!this->mEngine)
    {
      std::cerr << "Failed to create f3d::engine" << '\n';
      return;
    }

    this->mInteractor = &this->mEngine->getInteractor();

    this->mTimer = new QTimer(this);
    this->mTimer->setInterval(30);
    connect(this->mTimer, &QTimer::timeout, this, &F3DWidget::onTick);
    this->mTimer->start();

    this->mEngine->getScene().add(this->mFilePath);

    auto& win = this->mEngine->getWindow();
    win.setSize(this->width(), this->height());

    auto& opt = this->mEngine->getOptions();

    // Test setting an option works
    opt.render.grid.enable = true;
    opt.ui.axis = true;
  }

  void paintGL() override
  {
    this->mEngine->getWindow().render();
  }

private slots:
  void onTick()
  {
    if (!this->mEngine || !this->mInteractor)
    {
      return;
    }

    this->makeCurrent();
    this->mInteractor->triggerEventLoop(1.0 / 30.0);
    this->update();
    this->doneCurrent();
  }

private:
  std::string mFilePath;
  std::string mBaselinePath;
  std::string mOutputPath;
  std::unique_ptr<f3d::engine> mEngine;
  f3d::interactor* mInteractor = nullptr;
  QTimer* mTimer = nullptr;
};

class MainWindow : public QMainWindow
{
public:
  explicit MainWindow(std::string filePath, std::string baselinePath, std::string outputPath,
    QWidget* parent = nullptr)
    : QMainWindow(parent)
  {
    this->setWindowTitle(QStringLiteral("F3D QT External Window"));
    this->mViewer =
      new F3DWidget(std::move(filePath), std::move(baselinePath), std::move(outputPath), this);
    this->setCentralWidget(this->mViewer);
    this->resize(300, 300);
  }

public
  Q_SLOT : void onClose()
  {
    if (!this->mViewer->checkFrameBuffer())
    {
      QCoreApplication::exit(EXIT_FAILURE);
    }

    this->close();
  }

private:
  F3DWidget* mViewer;
};

int TestSDKExternalWindowQT(int argc, char** argv)
{
  QApplication app(argc, argv);

  MainWindow win(
    std::string(argv[1]) + "/data/cow.vtp", std::string(argv[1]) + "/baselines/", argv[2]);
  win.show();

  QTimer::singleShot(1000, &win, &MainWindow::onClose);

  return app.exec();
}
