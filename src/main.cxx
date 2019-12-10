#include "F3DLoader.h"
#include "F3DNSDelegate.h"
#include "F3DOffscreenRender.h"
#include "F3DOptions.h"

#include "vtkF3DInteractorStyle.h"
#include "vtkF3DRenderer.h"

#include <vtkAxesActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include "Config.h"

int main(int argc, char** argv)
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

  std::vector<std::string> files;

  F3DOptionsParser& parser = F3DOptionsParser::GetInstance();
  parser.Initialize(argc, argv);
  F3DOptions options = parser.GetOptionsFromCommandLine(files);

  vtkNew<vtkRenderWindow> renWin;
  renWin->SetSize(options.WindowSize[0], options.WindowSize[1]);
  renWin->SetWindowName(f3d::AppTitle.c_str());

  // the renderer must be added to the render window after OpenGL context initialization
  vtkNew<vtkF3DRenderer> ren;
  renWin->AddRenderer(ren);

  vtkNew<vtkRenderWindowInteractor> interactor;
  vtkNew<vtkF3DInteractorStyle> style;

  interactor->SetRenderWindow(renWin);
  interactor->SetInteractorStyle(style);
  interactor->Initialize();

#if __APPLE__
  F3DNSDelegate::InitializeDelegate(ren);
#endif

  ren->Initialize(options, "");

  if (files.size() > 0)
  {
    F3DLoader::GetInstance().AddFiles(files);
    F3DLoader::GetInstance().LoadCurrentIndex(ren);
  }

  int retVal = EXIT_SUCCESS;
  if (!options.Output.empty())
  {
    retVal = F3DOffscreenRender::RenderOffScreen(renWin, options.Output);
  }
  else if (!options.Reference.empty())
  {
    retVal = F3DOffscreenRender::RenderTesting(renWin, options.Reference, options.RefThreshold);
  }
  else
  {
    renWin->Render();
    interactor->Start();
  }

  // for some reason, the widget should be disable before destruction
  ren->ShowAxis(false);

  return retVal;
}

#if F3D_WIN32_APP
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  return main(__argc, __argv);
}
#endif
