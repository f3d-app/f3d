#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QQuickWindow>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QTimer>

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  // F3D requires OpenGL backend!
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  QCommandLineParser parser;
  parser.setApplicationDescription("libf3d + QML example");
  parser.addHelpOption();

  QCommandLineOption timeoutOpt("timeout", "Optional timeout (in seconds) before closing the viewer.", "seconds");
  parser.addOption(timeoutOpt);
  parser.addPositionalArgument("file", "3D model file to open");
  parser.process(app);

  const QStringList pos = parser.positionalArguments();
  if (pos.isEmpty())
  {
    qWarning("No model file provided");
    return 1;
  }

  QFileInfo fi(pos.first());
  if (!fi.exists())
  {
    qWarning() << "File not found:" << fi.absoluteFilePath();
    return 1;
  }

  bool okTimeout = false;
  int timeout = parser.value(timeoutOpt).toInt(&okTimeout);

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("fileArgument", fi.absoluteFilePath());
  engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));

  if (okTimeout && timeout > 0)
  {
    QTimer::singleShot(timeout * 1000, &app, &QCoreApplication::quit);
  }

  return app.exec();
}
