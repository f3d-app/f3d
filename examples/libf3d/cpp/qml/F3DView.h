#ifndef F3DVIEW_H
#define F3DVIEW_H

#include <QtQuick/QQuickFramebufferObject>

class F3DRenderer;

class F3DView : public QQuickFramebufferObject
{
  Q_OBJECT

  QML_ELEMENT

  Q_PROPERTY(QString modelPath READ modelPath WRITE setModelPath NOTIFY modelPathChanged)
  Q_PROPERTY(bool enableKeyboard MEMBER _enableKeyboard NOTIFY enableKeyboardChanged)
  Q_PROPERTY(bool enableMouse MEMBER _enableMouse NOTIFY enableMouseChanged)

public:
  explicit F3DView(QQuickItem* parent = nullptr);
  ~F3DView() override;

  Renderer* createRenderer() const override;
  void releaseResources() override;

  QString modelPath() const;
  void setModelPath(const QString& path);

  Q_INVOKABLE void setOption(const QString& opt, const QVariant& value);
  QVariantMap& pendingOptions();

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

signals:
  void modelPathChanged();
  void enableKeyboardChanged();
  void enableMouseChanged();

private:
  QString _modelPath;
  QVariantMap _pendingOptions;
  mutable F3DRenderer* _renderer = nullptr;

  bool _enableKeyboard = true;
  bool _enableMouse = true;
};

#endif // F3DVIEW_H
