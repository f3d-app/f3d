#include <engine.h>
#include <loader.h>
#include <window.h>

#include <QGuiApplication>
#include <QKeyEvent>
#include <QOpenGLWindow>

#include "TestSDKHelpers.h"

class F3DWindow : public QOpenGLWindow
{
public:
  F3DWindow(
    const std::string& filePath, const std::string& baselinePath, const std::string& outputPath)
    : QOpenGLWindow()
    , mEngine(f3d::window::Type::EXTERNAL)
    , mBaselinePath(baselinePath)
    , mOutputPath(outputPath)
  {
    f3d::loader& load = mEngine.getLoader();
    load.addFile(filePath).loadFile();
  }

protected:
  void keyPressEvent(QKeyEvent* event) override
  {
    if (event->key() == Qt::Key_Escape)
    {
      this->close();
    }
  }

  void timerEvent(QTimerEvent* event) override
  {
    // before closing, compare the content of the framebuffer with the baseline
    QImage frameBuffer = grabFramebuffer().mirrored().convertToFormat(QImage::Format_RGB888);

    f3d::image img;
    img.setResolution(frameBuffer.width(), frameBuffer.height())
      .setChannelCount(3)
      .setData(frameBuffer.bits());

    if (!TestSDKHelpers::RenderTest(
          img, this->mBaselinePath, this->mOutputPath, "TestSDKExternalWindowQT"))
    {
      QCoreApplication::exit(EXIT_FAILURE);
    }

    this->close();
  }

  void paintGL() override { mEngine.getWindow().render(); }

private:
  f3d::engine mEngine;
  std::string mBaselinePath;
  std::string mOutputPath;
};

int TestSDKExternalWindowQT(int argc, char* argv[])
{
  QGuiApplication a(argc, argv);

  F3DWindow w(
    std::string(argv[1]) + "/data/cow.vtp", std::string(argv[1]) + "/baselines/", argv[2]);
  w.setTitle("F3D QT External Window");
  w.resize(300, 300);
  w.startTimer(1000);

  w.show();

  return a.exec();
}
