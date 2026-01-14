#include "F3DView.h"
#include "F3DRenderer.h"
#include <QtCore/QSysInfo>
#include <QtGui/QWindow>
#include <f3d/engine.h>
#include <f3d/interactor.h>
#include <f3d/scene.h>

//----------------------------------------------------------------------------
F3DView::F3DView(QQuickItem* parent)
  : QQuickFramebufferObject(parent)
{
  setMirrorVertically(true);
  setAcceptedMouseButtons(Qt::AllButtons);
}

//----------------------------------------------------------------------------
F3DView::~F3DView()
{
  delete _renderer;
}

//----------------------------------------------------------------------------
QString F3DView::modelPath() const
{
  return _modelPath;
}

//----------------------------------------------------------------------------
void F3DView::setModelPath(const QString& path)
{
  if (_modelPath == path)
  {
    return;
  }

  _modelPath = path;
  emit modelPathChanged();
  update();
}

//----------------------------------------------------------------------------
QQuickFramebufferObject::Renderer* F3DView::createRenderer() const
{    
  _renderer = new F3DRenderer();
  return _renderer;
}

//----------------------------------------------------------------------------
void F3DView::releaseResources()
{
  delete _renderer;
  _renderer = nullptr;
}

//----------------------------------------------------------------------------
void F3DView::mousePressEvent(QMouseEvent *event)
{
  if (!_enableMouse)
  {
    return event->ignore();
  }
  if (_renderer)
  {
    _renderer->queueMousePress(event->position(), event->button(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::mouseMoveEvent(QMouseEvent *event)
{
  if (!_enableMouse)
  {
    return event->ignore();
  }
  if (_renderer)
  {
    _renderer->queueMouseMove(event->position(), event->button(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::mouseReleaseEvent(QMouseEvent *event)
{
  if (!_enableMouse)
  {
    return event->ignore();
  }
  if (_renderer)
  {
    _renderer->queueMouseRelease(event->position(), event->button(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::wheelEvent(QWheelEvent *event)
{
  if (!_enableMouse)
  {
    return event->ignore();
  }
  if (_renderer)
  {
    _renderer->queueWheel(event->angleDelta(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::keyPressEvent(QKeyEvent *event)
{
  if (!_enableKeyboard)
  {
    return event->ignore();
  }
  if (_renderer)
  {
    _renderer->queueKeyPress(event->key(), event->text(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::keyReleaseEvent(QKeyEvent *event)
{
  if (!_enableKeyboard)
  {
    return event->ignore();
  }
  if (_renderer)
  {
    _renderer->queueKeyRelease(event->key(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
  QQuickFramebufferObject::geometryChange(newGeometry, oldGeometry);
  update();
}

//----------------------------------------------------------------------------
QVariantMap& F3DView::pendingOptions()
{
  return _pendingOptions;
}

//----------------------------------------------------------------------------
void F3DView::setOption(const QString& opt, const QVariant& value)
{
  _pendingOptions.insert(opt, value);
  update();
}


