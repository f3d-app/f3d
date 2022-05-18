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
  F3DWindow(f3d::engine* engine)
    : QOpenGLWindow()
    , mEngine(engine)
  {
  }

protected:
  void resizeGL(int w, int h) override
  {
    mEngine->getOptions().set("resolution", { w, h });
    mEngine->getWindow().update();
  }

  void keyPressEvent(QKeyEvent* event) override
  {
    if (event->key() == Qt::Key_Escape)
    {
      close();
    }
  }

  void timerEvent(QTimerEvent* event) override
  {
    close();
  }

  void paintGL() override { mEngine->getWindow().render(); }

private:
  f3d::engine* mEngine;
};

int TestSDKExternalWindowQT(int argc, char* argv[])
{
  // create engine and load file
  f3d::engine eng(f3d::engine::CREATE_WINDOW | f3d::engine::WINDOW_EXTERNAL);
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/cow.vtp");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);

  QGuiApplication a(argc, argv);
  QSurfaceFormat fmt;
  fmt.setVersion(3, 2);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(fmt);

  F3DWindow w(&eng);
  w.setTitle("F3D QT External Window");
  w.resize(300, 300);
  w.startTimer(1000);

  w.show();

  return a.exec();
}
