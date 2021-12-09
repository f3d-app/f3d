#include "vtkF3DInteractionHandler.h"

#include <vtkBoundingBox.h>
#include <vtkInteractorStyle.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkStringArray.h>

#include "F3DAnimationManager.h"
#include "F3DIncludes.h"
#include "F3DLog.h"
#include "F3DLoader.h"
#include "F3DOptions.h"
#include "vtkF3DRendererWithColoring.h"


vtkStandardNewMacro(vtkF3DInteractionHandler);

//----------------------------------------------------------------------------
void vtkF3DInteractionHandler::SetupInteractorStyles(
  vtkRenderWindowInteractor* interactor, F3DAnimationManager* animManager, F3DOptions* options)
{
  this->Style3D->SetAnimationManager(*animManager);
  this->Style3D->SetInteractionHandler(this);
  // Will only be used when interacting with a animated file
  this->Style3D->SetOptions(*options);

  this->Style2D->SetAnimationManager(*animManager);
  this->Style2D->SetInteractionHandler(this);
  // Will only be used when interacting with a animated file
  this->Style2D->SetOptions(*options);

  interactor->SetInteractorStyle(this->Style2D);
  this->Interactor = interactor;
}

//----------------------------------------------------------------------------
void vtkF3DInteractionHandler::SetDefaultStyle(vtkRenderer* renderer)
{
  double bounds[6], lengths[3];
  renderer->ComputeVisiblePropBounds(bounds);
  vtkBoundingBox box(bounds);
  box.GetLengths(lengths);
  bool is2D = (lengths[0] == 0 || lengths[1] == 0 || lengths[2] == 0);
  if (is2D)
  {
    this->Interactor->SetInteractorStyle(this->Style2D);
  }
  else
  {
    this->Interactor->SetInteractorStyle(this->Style3D);
  }
}

//----------------------------------------------------------------------------
void vtkF3DInteractionHandler::OnDropFiles(vtkInteractorStyle* style, vtkStringArray* files)
{
  if (files == nullptr)
  {
    F3DLog::Print(F3DLog::Severity::Warning, "Drop event without any provided files.");
    return;
  }

  vtkRenderWindowInteractor* rwi = style->GetInteractor();
  vtkRenderWindow* renWin = rwi->GetRenderWindow();
  this->InvokeEvent(F3DLoader::NewFilesEvent, files);
  renWin->Render();
}

//----------------------------------------------------------------------------
bool vtkF3DInteractionHandler::HandleKeyPress(vtkInteractorStyle* style)
{
  vtkRenderWindowInteractor* rwi = style->GetInteractor();
  vtkRenderWindow* renWin = rwi->GetRenderWindow();
  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
  vtkF3DRendererWithColoring* renWithColor = vtkF3DRendererWithColoring::SafeDownCast(ren);

  switch (std::toupper(rwi->GetKeyCode()))
  {
    case 'C':
      if (renWithColor)
      {
        renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_FIELD_CYCLE);
        renWin->Render();
      }
      break;
    case 'S':
      if (renWithColor)
      {
        renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_ARRAY_CYCLE);
        renWin->Render();
      }
      break;
    case 'Y':
      if (renWithColor)
      {
        renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_COMPONENT_CYCLE);
        renWin->Render();
      }
      break;
    case 'B':
      if (renWithColor)
      {
        renWithColor->ShowScalarBar(!renWithColor->IsScalarBarVisible());
        renWin->Render();
      }
      break;
    case 'P':
      ren->SetUseDepthPeelingPass(!ren->UsingDepthPeelingPass());
      renWin->Render();
      break;
    case 'Q':
      ren->SetUseSSAOPass(!ren->UsingSSAOPass());
      renWin->Render();
      break;
    case 'A':
      ren->SetUseFXAAPass(!ren->UsingFXAAPass());
      renWin->Render();
      break;
    case 'T':
      ren->SetUseToneMappingPass(!ren->UsingToneMappingPass());
      renWin->Render();
      break;
    case 'E':
      ren->ShowEdge(!ren->IsEdgeVisible());
      renWin->Render();
      break;
    case 'X':
      ren->ShowAxis(!ren->IsAxisVisible());
      renWin->Render();
      break;
    case 'G':
      ren->ShowGrid(!ren->IsGridVisible());
      renWin->Render();
      break;
    case 'N':
      ren->ShowFilename(!ren->IsFilenameVisible());
      renWin->Render();
      break;
    case 'M':
      ren->ShowMetaData(!ren->IsMetaDataVisible());
      renWin->Render();
      break;
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
    case 'R':
      ren->SetUseRaytracing(!ren->UsingRaytracing());
      renWin->Render();
      break;
    case 'D':
      ren->SetUseRaytracingDenoiser(!ren->UsingRaytracingDenoiser());
      renWin->Render();
      break;
    case 'V':
      if (renWithColor)
      {
        renWithColor->SetUseVolume(!renWithColor->UsingVolume());
        renWin->Render();
      }
      break;
    case 'I':
      if (renWithColor)
      {
        renWithColor->SetUseInverseOpacityFunction(!renWithColor->UsingInverseOpacityFunction());
        renWin->Render();
      }
      break;
    case 'O':
      if (renWithColor)
      {
        renWithColor->SetUsePointSprites(!renWithColor->UsingPointSprites());
        renWin->Render();
      }
      break;
    case 'F':
      if (!renWin->GetFullScreen())
      {
        // save current window position and size
        int* pos = renWin->GetPosition();
        this->WindowPos[0] = pos[0];
        this->WindowPos[1] = pos[1];
        int* size = renWin->GetSize();
        this->WindowSize[0] = size[0];
        this->WindowSize[1] = size[1];
      }

      renWin->SetFullScreen(!renWin->GetFullScreen());

      if (!renWin->GetFullScreen() && this->WindowSize[0] > 0)
      {
        // restore previous window position and size
        renWin->SetPosition(this->WindowPos);
        renWin->SetSize(this->WindowSize);
      }

      // when going full screen, the OpenGL context changes, we need to reinitialize
      // the interactor, the render passes and the grid actor.
      ren->ShowGrid(ren->IsGridVisible());
      ren->SetupRenderPasses();
      rwi->ReInitialize();

      renWin->Render();
      break;
    case 'U':
      ren->SetUseBlurBackground(!ren->UsingBlurBackground());
      renWin->Render();
      break;
    case 'K':
      ren->SetUseTrackball(!ren->UsingTrackball());
      renWin->Render();
      break;
    case 'H':
      ren->ShowCheatSheet(!ren->IsCheatSheetVisible());
      renWin->Render();
      break;
    case '?':
      ren->DumpSceneState();
      break;
    case '2':
      this->Interactor->SetInteractorStyle(this->Style2D);
      break;
    case '3':
      this->Interactor->SetInteractorStyle(this->Style3D);
      break;
    default:
      std::string keySym = rwi->GetKeySym();
      if (keySym.length() > 0)
      {
        // Make sure key symbols starts with an upper char (e.g. "space")
        keySym[0] = std::toupper(keySym[0]);
      }
      if (keySym == "Left")
      {
        int load = F3DLoader::LOAD_PREVIOUS;
        style->InvokeEvent(F3DLoader::LoadFileEvent, &load);
        renWin->Render();
      }
      else if (keySym == "Right")
      {
        int load = F3DLoader::LOAD_NEXT;
        style->InvokeEvent(F3DLoader::LoadFileEvent, &load);
        renWin->Render();
      }
      else if (keySym == "Up")
      {
        int load = F3DLoader::LOAD_CURRENT;
        style->InvokeEvent(F3DLoader::LoadFileEvent, &load);
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
      else if (keySym == "Space")
      {
        style->InvokeEvent(F3DLoader::ToggleAnimationEvent);
        renWin->Render();
      }
      else
      {
        return false;
      }
      break;
  }
  return true;
}
