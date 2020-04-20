#include "vtkF3DInteractorStyle.h"

#include "F3DLoader.h"
#include "vtkF3DRenderer.h"

#include <vtkCallbackCommand.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkStringArray.h>


vtkStandardNewMacro(vtkF3DInteractorStyle);

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnDropFiles(vtkStringArray* files)
{
  vtkRenderWindowInteractor* rwi = this->GetInteractor();
  vtkRenderWindow* renWin = rwi->GetRenderWindow();
  this->InvokeEvent(F3DLoader::NewFilesEvent, files);
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
      ren->SetUseDepthPeelingPass(!ren->UsingDepthPeelingPass());
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
    case 'a':
    case 'A':
      ren->SetUseToneMappingPass(!ren->UsingToneMappingPass());
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
    case 'l':
    case 'L':
      renWin->SetFullScreen(!renWin->GetFullScreen());
      renWin->Render();
      rwi->ReInitialize();
      break;
    case 'z':
    case 'Z':
      ren->SetUseVolume(!ren->UsingVolume());
      renWin->Render();
      break;
    case 'i':
    case 'I':
      ren->SetUseInverseOpacityFunction(!ren->UsingInverseOpacityFunction());
      renWin->Render();
      break;
    default:
      std::string keySym = rwi->GetKeySym();
      if (keySym == "Left")
      {
        int load = F3DLoader::LOAD_PREVIOUS;
        this->InvokeEvent(F3DLoader::LoadFileEvent, &load);
        renWin->Render();
      }
      else if (keySym == "Right")
      {
        int load = F3DLoader::LOAD_NEXT;
        this->InvokeEvent(F3DLoader::LoadFileEvent, &load);
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

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::EnvironmentRotate()
{
  this->Superclass::EnvironmentRotate();

  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(this->CurrentRenderer);
  if (ren)
  {
    // update skybox orientation
    double* up = ren->GetEnvironmentUp();
    double* right = ren->GetEnvironmentRight();

    ren->GetSkybox()->SetFloorPlane(-up[2], up[1], up[0], 0.0);
    ren->GetSkybox()->SetFloorRight(-right[2], right[1], right[0]);

    this->Interactor->Render();
  }
}
