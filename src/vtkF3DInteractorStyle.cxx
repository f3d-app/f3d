#include "vtkF3DInteractorStyle.h"

#include "F3DAnimationManager.h"
#include "F3DIncludes.h"
#include "F3DLoader.h"
#include "F3DLog.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkStringArray.h>

vtkStandardNewMacro(vtkF3DInteractorStyle);

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::OnDropFiles(vtkStringArray* files)
{
  if (files == nullptr)
  {
    F3DLog::Print(F3DLog::Severity::Warning,
      "Drop event without any provided files.");
    return;
  }

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
  vtkF3DRendererWithColoring* renWithColor = vtkF3DRendererWithColoring::SafeDownCast(ren);

  switch (rwi->GetKeyCode())
  {
    case 'c':
    case 'C':
      if (renWithColor)
      {
        renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_FIELD_CYCLE);
        renWin->Render();
      }
      break;
    case 's':
    case 'S':
      if (renWithColor)
      {
        renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_ARRAY_CYCLE);
        renWin->Render();
      }
      break;
    case 'y':
    case 'Y':
      if (renWithColor)
      {
        renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_COMPONENT_CYCLE);
        renWin->Render();
      }
      break;
    case 'b':
    case 'B':
      if (renWithColor)
      {
        renWithColor->ShowScalarBar(!renWithColor->IsScalarBarVisible());
        renWin->Render();
      }
      break;
    case 'p':
    case 'P':
      ren->SetUseDepthPeelingPass(!ren->UsingDepthPeelingPass());
      renWin->Render();
      break;
    case 'q':
    case 'Q':
      ren->SetUseSSAOPass(!ren->UsingSSAOPass());
      renWin->Render();
      break;
    case 'a':
    case 'A':
      ren->SetUseFXAAPass(!ren->UsingFXAAPass());
      renWin->Render();
      break;
    case 't':
    case 'T':
      ren->SetUseToneMappingPass(!ren->UsingToneMappingPass());
      renWin->Render();
      break;
    case 'e':
    case 'E':
      ren->ShowEdge(!ren->IsEdgeVisible());
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
    case 'n':
    case 'N':
      ren->ShowFilename(!ren->IsFilenameVisible());
      renWin->Render();
      break;
    case 'm':
    case 'M':
      ren->ShowMetaData(!ren->IsMetaDataVisible());
      renWin->Render();
      break;
    case 'z':
    case 'Z':
      ren->ShowTimer(!ren->IsTimerVisible());
      renWin->Render();
      if (ren->IsTimerVisible())
      {
        // Needed to show a correct value, computed at the previous render
        // TODO: Improve this by computing it and displaying it within a single render
        renWin->Render();
      }
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
    case 'v':
    case 'V':
      if (renWithColor)
      {
        renWithColor->SetUseVolume(!renWithColor->UsingVolume());
        renWin->Render();
      }
      break;
    case 'i':
    case 'I':
      if (renWithColor)
      {
        renWithColor->SetUseInverseOpacityFunction(!renWithColor->UsingInverseOpacityFunction());
        renWin->Render();
      }
      break;
    case 'o':
    case 'O':
      if (renWithColor)
      {
        renWithColor->SetUsePointSprites(!renWithColor->UsingPointSprites());
        renWin->Render();
      }
      break;
    case 'f':
    case 'F':
      renWin->SetFullScreen(!renWin->GetFullScreen());

      // when going full screen, the OpenGL context changes, we need to reinitialize
      // the interactor, the render passes and the grid actor.
      ren->ShowGrid(ren->IsGridVisible());
      ren->SetupRenderPasses();
      rwi->ReInitialize();

      renWin->Render();
      break;
    case 'u':
    case 'U':
      ren->SetUseBlurBackground(!ren->UsingBlurBackground());
      renWin->Render();
      break;
    case 'k':
    case 'K':
      ren->SetUseTrackball(!ren->UsingTrackball());
      renWin->Render();
      break;
    case 'h':
    case 'H':
      ren->ShowCheatSheet(!ren->IsCheatSheetVisible());
      renWin->Render();
      break;
    case '?':
      ren->DumpSceneState();
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
      else if (keySym == "Up")
      {
        int load = F3DLoader::LOAD_CURRENT;
        this->InvokeEvent(F3DLoader::LoadFileEvent, &load);
        renWin->Render();
      }
      else if (keySym == F3D::EXIT_HOTKEY_SYM)
      {
        rwi->RemoveObservers(vtkCommand::TimerEvent);
        rwi->ExitCallback();
      }
      else if (keySym == "Return")
      {
        ren->ResetCamera();
        renWin->Render();
      }
      else if (keySym == "space")
      {
        this->InvokeEvent(F3DLoader::ToggleAnimationEvent);
        renWin->Render();
      }
      break;
  }
}

//------------------------------------------------------------------------------
void vtkF3DInteractorStyle::Rotate()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }

  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(this->CurrentRenderer);

  if (ren == nullptr)
  {
    return;
  }

  vtkRenderWindowInteractor* rwi = this->Interactor;

  int dx = rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
  int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

  const int* size = ren->GetRenderWindow()->GetSize();

  double delta_elevation = -20.0 / size[1];
  double delta_azimuth = -20.0 / size[0];

  double rxf = dx * delta_azimuth * this->MotionFactor;
  double ryf = dy * delta_elevation * this->MotionFactor;

  vtkCamera* camera = ren->GetActiveCamera();
  double dir[3];
  camera->GetDirectionOfProjection(dir);
  double* up = ren->GetUpVector();

  double dot = vtkMath::Dot(dir, up);

  bool canElevate = ren->UsingTrackball() || std::abs(dot) < 0.99 || !std::signbit(dot * ryf);

  camera->Azimuth(rxf);

  if (canElevate)
  {
    camera->Elevation(ryf);
  }

  if (!ren->UsingTrackball())
  {
    // orthogonalize up vector based on focal direction
    vtkMath::MultiplyScalar(dir, dot);
    vtkMath::Subtract(up, dir, dir);
    vtkMath::Normalize(dir);
    camera->SetViewUp(dir);
  }
  else
  {
    camera->OrthogonalizeViewUp();
  }

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }

  if (rwi->GetLightFollowCamera())
  {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
  }

  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::Spin()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Spin();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::Pan()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Pan();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::Dolly()
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Dolly();
}

//----------------------------------------------------------------------------
void vtkF3DInteractorStyle::Dolly(double factor)
{
  if (this->IsUserInteractionBlocked())
  {
    return;
  }
  this->Superclass::Dolly(factor);
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

    double front[3];
    vtkMath::Cross(right, up, front);

    ren->GetSkybox()->SetFloorPlane(up[0], up[1], up[2], 0.0);
    ren->GetSkybox()->SetFloorRight(front[0], front[1], front[2]);

    this->Interactor->Render();
  }
}

//----------------------------------------------------------------------------
bool vtkF3DInteractorStyle::IsUserInteractionBlocked()
{
  return this->AnimationManager->IsPlaying() && this->Options->CameraIndex >= 0;
}
