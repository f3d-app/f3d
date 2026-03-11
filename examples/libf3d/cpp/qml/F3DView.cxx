#include "F3DView.h"

#include "F3DRenderer.h"

#include <QtCore/QSysInfo>
#include <QtGui/QWindow>

#include <f3d/interactor.h>
#include <f3d/scene.h>

//----------------------------------------------------------------------------
F3DView::F3DView(QQuickItem* parent)
  : QQuickFramebufferObject(parent)
{
}

//----------------------------------------------------------------------------
QString F3DView::GetModelPath() const
{
  return this->modelPath;
}

//----------------------------------------------------------------------------
void F3DView::SetModelPath(const QString& path)
{
  if (this->modelPath == path)
  {
    return;
  }

  this->modelPath = path;
  emit this->ModelPathChanged();
}

//----------------------------------------------------------------------------
QQuickFramebufferObject::Renderer* F3DView::createRenderer() const
{
  auto* renderer = new F3DRenderer(const_cast<F3DView*>(this));
  this->Renderer = renderer;
  return renderer;
}

//----------------------------------------------------------------------------
void F3DView::releaseResources()
{
  this->Renderer = nullptr;
}

//----------------------------------------------------------------------------
void F3DView::MousePress(float x, float y, int button, int modifiers)
{
  if (this->Renderer != nullptr)
  {
    this->Renderer->QueueMousePress(x, y, button, modifiers);
  }
}

//----------------------------------------------------------------------------
void F3DView::MouseMove(float x, float y, int buttons, int modifiers)
{
  if (this->Renderer != nullptr)
  {
    this->Renderer->QueueMouseMove(x, y, buttons, modifiers);
  }
}

//----------------------------------------------------------------------------
void F3DView::MouseRelease(float x, float y, int button, int modifiers)
{
  if (this->Renderer != nullptr)
  {
    this->Renderer->QueueMouseRelease(x, y, button, modifiers);
  }
}

//----------------------------------------------------------------------------
void F3DView::MouseWheel(int dx, int dy, int modifiers)
{
  if (this->Renderer != nullptr)
  {
    this->Renderer->QueueWheel(dx, dy, modifiers);
  }
}

//----------------------------------------------------------------------------
void F3DView::KeyPress(int key, const QString& text, int modifiers)
{
  if (this->Renderer != nullptr)
  {
    this->Renderer->QueueKeyPress(key, text, modifiers);
  }
}

//----------------------------------------------------------------------------
void F3DView::KeyRelease(int key, int modifiers)
{
  if (this->Renderer != nullptr)
  {
    this->Renderer->QueueKeyRelease(key, modifiers);
  }
}
