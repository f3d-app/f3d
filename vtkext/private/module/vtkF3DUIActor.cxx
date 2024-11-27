#include "vtkF3DUIActor.h"

#include <vtkObjectFactory.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkViewport.h>

vtkObjectFactoryNewMacro(vtkF3DUIActor);

//----------------------------------------------------------------------------
vtkF3DUIActor::vtkF3DUIActor() = default;

//----------------------------------------------------------------------------
vtkF3DUIActor::~vtkF3DUIActor() = default;

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetFileNameVisibility(bool show)
{
  this->FileNameVisible = show;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetFileName(const std::string& filename)
{
  this->FileName = filename;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetFontFile(const std::string& font)
{
  this->FontFile = font;
}

//----------------------------------------------------------------------------
int vtkF3DUIActor::RenderOverlay(vtkViewport* vp)
{
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(vp->GetVTKWindow());
  this->StartFrame(renWin);

  if (this->FileNameVisible)
  {
    this->RenderFileName();
  }

  this->EndFrame(renWin);

  return 1;
}
