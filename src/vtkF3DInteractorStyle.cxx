#include "vtkF3DInteractorStyle.h"

#include "F3DLoader.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkStringArray.h>

#include "vtkF3DRenderer.h"

vtkStandardNewMacro(vtkF3DInteractorStyle);

void vtkF3DInteractorStyle::OnDropFiles(vtkStringArray* files)
{
  vtkRenderWindowInteractor* rwi = this->GetInteractor();
  vtkRenderWindow* renWin = rwi->GetRenderWindow();
  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());

  for (int i = 0; i < files->GetNumberOfTuples(); i++)
  {
    F3DLoader::GetInstance().AddFile(files->GetValue(i), ren);
  }

  F3DLoader::GetInstance().LoadNext(ren);
  renWin->Render();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnKeyPress()
{
  vtkRenderWindowInteractor* rwi = this->GetInteractor();
  vtkRenderWindow* renWin = rwi->GetRenderWindow();
  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());

  switch (rwi->GetKeyCode())
  {
    case 's':
    case 'S':
      ren->ShowScalars(!ren->AreScalarsVisible());
      renWin->Render();
      break;
    case 'r':
    case 'R':
      ren->SetUseRaytracing(!ren->UsingRaytracing());
      renWin->Render();
      break;
    case 'd':
    case 'D':
      ren->SetUseRaytracingDenoiser(!ren->UsingRaytracingDenoiser());
      renWin->Render();
      break;
    case 'p':
    case 'P':
      ren->SetUseDepthPeeling(!ren->UsingDepthPeeling());
      renWin->Render();
      break;
    case 'u':
    case 'U':
      ren->SetUseSSAOPass(!ren->UsingSSAOPass());
      renWin->Render();
      break;
    case 'f':
    case 'F':
      ren->SetUseFXAAPass(!ren->UsingFXAAPass());
      renWin->Render();
      break;
    case 'o':
    case 'O':
      ren->SetUsePointSprites(!ren->UsingPointSprites());
      renWin->Render();
      break;
    case 't':
    case 'T':
      ren->ShowTimer(!ren->IsTimerVisible());
      renWin->Render();
      break;
    case 'x':
    case 'X':
      ren->ShowAxis(!ren->IsAxisVisible());
      renWin->Render();
      break;
    case 'g':
    case 'G':
      ren->ShowGrid(!ren->IsGridVisible());
      renWin->Render();
      break;
    case 'b':
    case 'B':
      ren->ShowScalarBar(!ren->IsScalarBarVisible());
      renWin->Render();
      break;
    case 'e':
    case 'E':
      ren->ShowEdge(!ren->IsEdgeVisible());
      renWin->Render();
      break;
    case 'n':
    case 'N':
      ren->ShowFilename(!ren->IsFilenameVisible());
      renWin->Render();
      break;
    default:
      std::string keySym = rwi->GetKeySym();
      if (keySym == "Left")
      {
        F3DLoader::GetInstance().LoadPrevious(ren);
        renWin->Render();
      }
      else if (keySym == "Right")
      {
        F3DLoader::GetInstance().LoadNext(ren);
        renWin->Render();
      }
      else if (keySym == "Escape")
      {
        rwi->ExitCallback();
      }
      else if (keySym == "Return")
      {
        ren->ResetCamera();
        renWin->Render();
      }
      break;
  }
}
