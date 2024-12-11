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
void vtkF3DUIActor::SetMetaDataVisibility(bool show)
{
  this->MetaDataVisible = show;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetMetaData(const std::string& metadata)
{
  this->MetaData = metadata;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetCheatSheetVisibility(bool show)
{
  this->CheatSheetVisible = show;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetConsoleVisibility(bool show)
{
  this->ConsoleVisible = show;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetCheatSheet(const std::vector<CheatSheetGroup>& cheatsheet)
{
  this->CheatSheet = cheatsheet;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetFpsCounterVisibility(bool show)
{
  this->FpsCounterVisible = show;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetFpsValue(int fps)
{
  this->FpsValue = fps;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetFontFile(const std::string& font)
{
  if (this->FontFile != font)
  {
    this->FontFile = font;
    this->Initialized = false;
  }
}

//----------------------------------------------------------------------------
int vtkF3DUIActor::RenderOverlay(vtkViewport* vp)
{
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(vp->GetVTKWindow());

  if (!this->Initialized)
  {
    this->Initialize(renWin);
    this->Initialized = true;
  }

  this->StartFrame(renWin);

  if (this->FileNameVisible)
  {
    this->RenderFileName();
  }

  if (this->MetaDataVisible)
  {
    this->RenderMetaData();
  }

  if (this->CheatSheetVisible)
  {
    this->RenderCheatSheet();
  }

  if (this->FpsCounterVisible)
  {
    this->RenderFpsCounter();
  }

  if (this->ConsoleVisible)
  {
    this->RenderConsole();
  }
  else
  {
    this->RenderConsoleBadge();
  }

  this->EndFrame(renWin);

  return 1;
}
