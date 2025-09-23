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
void vtkF3DUIActor::SetDropZoneVisibility(bool show)
{
  this->DropZoneVisible = show;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetDropZoneLogoVisibility(bool show)
{
  this->DropZoneLogoVisible = show;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetDropText(const std::string& info)
{
  this->DropText = info;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetDropBinds(
  const std::vector<std::pair<std::string, std::string>>& dropZoneBinds)
{
  this->DropBinds = dropZoneBinds;
}

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
void vtkF3DUIActor::SetMinimalConsoleVisibility(bool show)
{
  this->MinimalConsoleVisible = show;
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetConsoleBadgeEnabled(bool enabled)
{
  this->ConsoleBadgeEnabled = enabled;
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
void vtkF3DUIActor::UpdateFpsValue(const double elapsedFrameTime)
{
  this->TotalFrameTimes += elapsedFrameTime;
  this->FrameTimes.push_back(elapsedFrameTime);

  while (this->TotalFrameTimes > 1.0)
  {
    double oldestFrameTime = this->FrameTimes.front();

    this->FrameTimes.pop_front();
    this->TotalFrameTimes -= oldestFrameTime;
  }

  double averageFrameTime = this->TotalFrameTimes / this->FrameTimes.size();
  this->FpsValue = static_cast<int>(std::round(1.0 / averageFrameTime));
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
void vtkF3DUIActor::SetFontScale(const double fontScale)
{
  if (this->FontScale != fontScale)
  {
    this->FontScale = fontScale;
    this->Initialized = false;
  }
}

//----------------------------------------------------------------------------
void vtkF3DUIActor::SetBackdropOpacity(const double backdropOpacity)
{
  if (this->BackdropOpacity != backdropOpacity)
  {
    this->BackdropOpacity = backdropOpacity;
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

  if (this->DropZoneVisible)
  {
    this->RenderDropZone();
  }

  if (this->ConsoleVisible)
  {
    // To improve user readability when console is visible, all other overlays won't be shown
    this->RenderConsole(false);
    this->EndFrame(renWin);
    return 1;
  }

  if (this->MinimalConsoleVisible)
  {
    // To improve user readability when minimal console is visible cheatsheet and filename
    // are not shown
    this->RenderConsole(true);
  }
  else
  {
    if (this->FileNameVisible)
    {
      this->RenderFileName();
    }
    if (this->CheatSheetVisible)
    {
      this->RenderCheatSheet();
    }
  }

  if (this->ConsoleBadgeEnabled)
  {
    this->RenderConsoleBadge();
  }

  if (this->MetaDataVisible)
  {
    this->RenderMetaData();
  }

  if (this->FpsCounterVisible)
  {
    this->RenderFpsCounter();
  }

  this->EndFrame(renWin);

  return 1;
}
