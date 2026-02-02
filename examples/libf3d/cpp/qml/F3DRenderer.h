#ifndef F3DRENDERER_H
#define F3DRENDERER_H

#include <QElapsedTimer>
#include <QSizeF>
#include <QString>
#include <QtQuick/QQuickFramebufferObject>

#include <deque>
#include <memory>
#include <mutex>

namespace f3d
{
class engine;
class interactor;
}

class F3DRenderer : public QQuickFramebufferObject::Renderer
{

public:
  explicit F3DRenderer();
  ~F3DRenderer() override;

  void queueMousePress(QPointF position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
  void queueMouseMove(QPointF position, Qt::MouseButton buttons, Qt::KeyboardModifiers modifiers);
  void queueMouseRelease(QPointF position, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
  void queueWheel(QPoint angleDelta, Qt::KeyboardModifiers modifiers);
  void queueKeyPress(int key, const QString& text, Qt::KeyboardModifiers modifiers);
  void queueKeyRelease(int key, Qt::KeyboardModifiers modifiers);

  void render() override;
  void synchronize(QQuickFramebufferObject* item) override;

  void updateSize(const QSizeF& size);

private:
  enum class EventType
  {
    MousePress,
    MouseMove,
    MouseRelease,
    Wheel,
    KeyPress,
    KeyRelease
  };

  struct Event
  {
    EventType type;
    double x = 0.0;
    double y = 0.0;
    Qt::MouseButton button = Qt::NoButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    int dx = 0;
    int dy = 0;
    int key = 0;
    QString text;
  };

  void initialize();
  void enqueue(const Event& ev);

  void updateModifiers(Qt::KeyboardModifiers mods);
  int mapMouseButton(Qt::MouseButton button) const;
  std::string keySymFromKeyAndText(int key, const QString& text) const;
  void handleEvent(const Event& ev);

  std::unique_ptr<f3d::engine> _engine;
  f3d::interactor* _interactor = nullptr;

  std::deque<Event> _events;
  std::mutex _mutex;
  QElapsedTimer _frameTimer;

  QSizeF _lastSize;
  QString _lastModelPath;
};

#endif // F3DRENDERER_H
