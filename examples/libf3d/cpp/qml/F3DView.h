#ifndef F3DVIEW_H
#define F3DVIEW_H

#include <QtQuick/QQuickFramebufferObject>

class F3DRenderer;

class F3DView : public QQuickFramebufferObject
{
  Q_OBJECT
  Q_PROPERTY(QString modelPath READ GetModelPath WRITE SetModelPath NOTIFY ModelPathChanged)

public:
  explicit F3DView(QQuickItem* parent = nullptr);

  QString GetModelPath() const;
  void SetModelPath(const QString& path);

  Renderer* createRenderer() const override;
  void releaseResources() override;

public slots:
  void MousePress(float x, float y, int button, int modifiers);
  void MouseMove(float x, float y, int buttons, int modifiers);
  void MouseRelease(float x, float y, int button, int modifiers);

  void MouseWheel(int dx, int dy, int modifiers);

  void KeyPress(int key, const QString& text, int modifiers);
  void KeyRelease(int key, int modifiers);

signals:
  void ModelPathChanged();

private:
  QString modelPath;
  mutable F3DRenderer* Renderer = nullptr;
};

#endif // F3DVIEW_H
