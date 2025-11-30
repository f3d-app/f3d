#ifndef F3DRENDERER_H
#define F3DRENDERER_H

#include <QElapsedTimer>
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

class F3DView;

class F3DRenderer : public QQuickFramebufferObject::Renderer
{
public:
  explicit F3DRenderer(F3DView* item);
  ~F3DRenderer() override;

  void QueueMousePress(float x, float y, int button, int modifiers);
  void QueueMouseMove(float x, float y, int buttons, int modifiers);
  void QueueMouseRelease(float x, float y, int button, int modifiers);
  void QueueWheel(int dx, int dy, int modifiers);
  void QueueKeyPress(int key, const QString& text, int modifiers);
  void QueueKeyRelease(int key, int modifiers);

  void render() override;

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
    int button = 0;
    int buttons = 0;
    int modifiers = 0;
    int dx = 0;
    int dy = 0;
    int key = 0;
    QString text;
  };

  void Enqueue(const Event& ev);

  void UpdateModifiers(int mods);
  int MapMouseButton(int button) const;
  std::string KeySymFromKeyAndText(int key, const QString& text) const;
  void HandleEvent(const Event& ev);

  F3DView* View = nullptr;
  std::unique_ptr<f3d::engine> Engine;
  f3d::interactor* Interactor = nullptr;

  std::deque<Event> Events;
  std::mutex Mutex;
  QElapsedTimer FrameTimer;
  bool FirstFrame = true;
};

#endif // F3DRENDERER_H
