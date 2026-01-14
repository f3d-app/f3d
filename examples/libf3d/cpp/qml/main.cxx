#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QQuickWindow>

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  qInfo()<<"F3D requires OpenGL backend";
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  QQmlApplicationEngine engine;
  engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

  return app.exec();
}
