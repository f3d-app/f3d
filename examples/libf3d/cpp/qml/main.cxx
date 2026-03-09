#include <QCommandLineParser>
#include <QDebug>
#include <QFileInfo>
#include <QGuiApplication>
#include <QTimer>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include "F3DView.h"

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);
  QCoreApplication::setApplicationName("libf3d + QML example");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("libf3d + QML example");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption timeoutOpt(
    "timeout", "Optional timeout (in seconds) before closing the viewer.", "seconds");
  parser.addOption(timeoutOpt);

  parser.addPositionalArgument("file", "3D model file to open");

  parser.process(app);

  const QStringList pos = parser.positionalArguments();
  if (pos.isEmpty())
  {
    qWarning("No model file provided");
    return 1;
  }

  QString filePath = pos.first();
  QFileInfo fi(filePath);
  if (!fi.exists())
  {
    qWarning() << "File not found:" << fi.absoluteFilePath();
    return 1;
  }

  bool okTimeout = false;
  int timeout = parser.value(timeoutOpt).toInt(&okTimeout);

  qmlRegisterType<F3DView>("F3D", 1, 0, "F3DView");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("initialModelPath", fi.absoluteFilePath());

  const QUrl url = QUrl::fromLocalFile("Main.qml");
  QObject::connect(
    &engine, &QQmlApplicationEngine::objectCreated, &app,
    [url](QObject* obj, const QUrl& objUrl)
    {
      if (!obj && url == objUrl)
      {
        QCoreApplication::exit(-1);
      }
    },
    Qt::QueuedConnection);

  engine.load(url);

  if (engine.rootObjects().isEmpty())
  {
    return 1;
  }

  if (okTimeout && timeout > 0)
  {
    QTimer::singleShot(timeout * 1000, &app, &QCoreApplication::quit);
  }

  return app.exec();
}
