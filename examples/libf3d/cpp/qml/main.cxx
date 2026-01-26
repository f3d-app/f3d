#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QQuickWindow>

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  if(argc < 2){
    qWarning()<<"File argument required";
    exit(1);
  }
  QString fileArg(argv[1]);

  // F3D requires OpenGL backend!
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  QQmlApplicationEngine engine;

  engine.rootContext()->setContextProperty("fileArgument", fileArg);
  engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

  return app.exec();
}
