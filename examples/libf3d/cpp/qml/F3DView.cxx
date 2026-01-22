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
  this->setMirrorVertically(true);
  this->setAcceptedMouseButtons(Qt::AllButtons);
}

//----------------------------------------------------------------------------
F3DView::~F3DView() = default;

//----------------------------------------------------------------------------
QString F3DView::modelPath() const
{
  return this->_modelPath;
}

//----------------------------------------------------------------------------
void F3DView::setModelPath(const QString& path)
{
  if (this->_modelPath == path)
  {
    return;
  }

  this->_modelPath = path;
  emit this->modelPathChanged();
  this->update();
}

//----------------------------------------------------------------------------
QQuickFramebufferObject::Renderer* F3DView::createRenderer() const
{
  // Note that the renderer is managed by QQuickFramebufferObject, do not delete this manually.
  this->_renderer = new F3DRenderer();
  return this->_renderer;
}

//----------------------------------------------------------------------------
void F3DView::releaseResources()
{
  // delete this->_renderer;
  // this->_renderer = nullptr;
}

//----------------------------------------------------------------------------
void F3DView::mousePressEvent(QMouseEvent *event)
{
  if (!this->_enableMouse)
  {
    return event->ignore();
  }
  if (this->_renderer)
  {
    this->_renderer->queueMousePress(event->position(), event->button(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::mouseMoveEvent(QMouseEvent *event)
{
  if (!this->_enableMouse)
  {
    return event->ignore();
  }
  if (this->_renderer)
  {
    this->_renderer->queueMouseMove(event->position(), event->button(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::mouseReleaseEvent(QMouseEvent *event)
{
  if (!this->_enableMouse)
  {
    return event->ignore();
  }
  if (this->_renderer)
  {
    this->_renderer->queueMouseRelease(event->position(), event->button(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::wheelEvent(QWheelEvent *event)
{
  if (!this->_enableMouse)
  {
    return event->ignore();
  }
  if (this->_renderer)
  {
    this->_renderer->queueWheel(event->angleDelta(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::keyPressEvent(QKeyEvent *event)
{
  if (!this->_enableKeyboard)
  {
    return event->ignore();
  }
  if (this->_renderer)
  {
    this->_renderer->queueKeyPress(event->key(), event->text(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::keyReleaseEvent(QKeyEvent *event)
{
  if (!this->_enableKeyboard)
  {
    return event->ignore();
  }
  if (this->_renderer)
  {
    this->_renderer->queueKeyRelease(event->key(), event->modifiers());
  }
}

//----------------------------------------------------------------------------
void F3DView::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
  QQuickFramebufferObject::geometryChange(newGeometry, oldGeometry);
  this->update();
}

//----------------------------------------------------------------------------
QVariantMap& F3DView::pendingOptions()
{
  return this->_pendingOptions;
}

//----------------------------------------------------------------------------
void F3DView::setOption(const QString& opt, const QVariant& value)
{
  this->_pendingOptions.insert(opt, value);
  this->update();
}


