#include <engine.h>
#include <loader.h>
#include <options.h>
#include <window.h>

#include <QGuiApplication>
#include <QKeyEvent>
#include <QOpenGLWindow>

class F3DWindow : public QOpenGLWindow
{
public:
  F3DWindow(const std::string& filePath, const std::string& baseline)
    : QOpenGLWindow()
    , mEngine(f3d::engine::CREATE_WINDOW | f3d::engine::WINDOW_EXTERNAL)
    , mBaseline(baseline)
  {
    f3d::loader& load = mEngine.getLoader();
    load.addFile(filePath);
    load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  }

protected:
  void resizeGL(int w, int h) override
  {
    mEngine.getOptions().set("resolution", { w, h });
    mEngine.getWindow().update();
  }

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

    f3d::image diff;
    double error;
    if (!img.compare(f3d::image(mBaseline), diff, 50, error))
    {
      QCoreApplication::exit(EXIT_FAILURE);
    }

    this->close();
  }

  void paintGL() override { mEngine.getWindow().render(); }

private:
  f3d::engine mEngine;
  std::string mBaseline;
};

int TestSDKExternalWindowQT(int argc, char* argv[])
{
  QGuiApplication a(argc, argv);

  F3DWindow w(std::string(argv[1]) + "/data/cow.vtp",
    std::string(argv[1]) + "/baselines/TestSDKExternalWindowQT.png");
  w.setTitle("F3D QT External Window");
  w.resize(300, 300);
  w.startTimer(1000);

  w.show();

  return a.exec();
}
