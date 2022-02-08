#include "f3d_interactor.h"

#include "F3DAnimationManager.h"
#include "F3DConfig.h"
#include "F3DLog.h"
#include "f3d_loader.h"
#include "f3d_window.h"
#include "vtkF3DInteractorEventRecorder.h"
#include "vtkF3DInteractorStyle.h"
#include "vtkF3DRendererWithColoring.h"

#include <vtkCallbackCommand.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>

#include <map>

namespace f3d
{
class interactor::F3DInternals
{
public:
  F3DInternals()
  {
    vtkNew<vtkCallbackCommand> keyPressCallback;
    keyPressCallback->SetClientData(this);
    keyPressCallback->SetCallback(OnKeyPress);
    this->Style->AddObserver(vtkF3DInteractorStyle::KeyPressEvent, keyPressCallback);

    vtkNew<vtkCallbackCommand> dropFilesCallback;
    dropFilesCallback->SetClientData(this);
    dropFilesCallback->SetCallback(OnDropFiles);
    this->Style->AddObserver(vtkF3DInteractorStyle::DropFilesEvent, dropFilesCallback);

    this->Recorder->SetInteractor(this->Interactor);
  }

  static void OnKeyPress(vtkObject*, unsigned long, void* clientData, void*)
  {
    F3DInternals* self = static_cast<F3DInternals*>(clientData);
    vtkRenderWindowInteractor* rwi = self->Style->GetInteractor();
    int keyCode = std::toupper(rwi->GetKeyCode());
    std::string keySym = rwi->GetKeySym();
    if (keySym.length() > 0)
    {
      // Make sure key symbols starts with an upper char (e.g. "space")
      keySym[0] = std::toupper(keySym[0]);
    }

    if (self->KeyPressUserCallBack(keyCode, keySym))
    {
      return;
    }

    // No user defined behavior, use standard behavior
    vtkRenderWindow* renWin = rwi->GetRenderWindow();
    vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
    vtkF3DRendererWithColoring* renWithColor = vtkF3DRendererWithColoring::SafeDownCast(ren);

    switch (keyCode)
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
      case 'p':
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
          self->WindowPos[0] = pos[0];
          self->WindowPos[1] = pos[1];
          int* size = renWin->GetSize();
          self->WindowSize[0] = size[0];
          self->WindowSize[1] = size[1];
        }

        renWin->SetFullScreen(!renWin->GetFullScreen());

        if (!renWin->GetFullScreen() && self->WindowSize[0] > 0)
        {
          // restore previous window position and size
          renWin->SetPosition(self->WindowPos);
          renWin->SetSize(self->WindowSize);
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
      default:
        if (keySym == "Left")
        {
          self->AnimationManager.StopAnimation();
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_PREVIOUS;
          self->Loader->loadFile(load);
          renWin->Render();
        }
        else if (keySym == "Right")
        {
          self->AnimationManager.StopAnimation();
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_NEXT;
          self->Loader->loadFile(load);
          renWin->Render();
        }
        else if (keySym == "Up")
        {
          self->AnimationManager.StopAnimation();
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_CURRENT;
          self->Loader->loadFile(load);
          renWin->Render();
        }
        else if (keySym == f3d::EXIT_HOTKEY_SYM)
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
          self->AnimationManager.ToggleAnimation();
          renWin->Render();
        }
        break;
    }
  }

  static void OnDropFiles(vtkObject*, unsigned long, void* clientData, void* callData)
  {
    F3DInternals* self = static_cast<F3DInternals*>(clientData);
    vtkStringArray* filesArr = static_cast<vtkStringArray*>(callData);
    std::vector<std::string> filesVec;
    for (int i = 0; i < filesArr->GetNumberOfTuples(); i++)
    {
      filesVec.push_back(filesArr->GetValue(i));
    }

    if (self->DropFilesUserCallBack(filesVec))
    {
      return;
    }

    // No user defined behavior, use standard behavior
    self->AnimationManager.StopAnimation();
    for (std::string file : filesVec)
    {
      self->Loader->addFile(file);
    }
    self->Loader->loadFile(f3d::loader::LoadFileEnum::LOAD_LAST);
  }

  std::function<bool(int, std::string)> KeyPressUserCallBack = [](int, std::string)
  { return false; };
  std::function<bool(std::vector<std::string>)> DropFilesUserCallBack = [](std::vector<std::string>)
  { return false; };

  F3DAnimationManager AnimationManager;
  vtkNew<vtkRenderWindowInteractor> Interactor;
  vtkNew<vtkF3DInteractorStyle> Style;
  vtkNew<vtkF3DInteractorEventRecorder> Recorder;
  f3d::loader* Loader;
  int WindowSize[2] = { -1, -1 };
  int WindowPos[2] = { 0, 0 };
  std::map<unsigned long, std::pair<int, std::function<void()> > > TimerCallBacks;
};

//----------------------------------------------------------------------------
interactor::interactor()
  : Internals(new interactor::F3DInternals)
{
}

//----------------------------------------------------------------------------
interactor::~interactor() = default;

//----------------------------------------------------------------------------
void interactor::SetLoader(f3d::loader* loader)
{
  this->Internals->Loader = loader;
  this->Internals->Interactor->SetRenderWindow(loader->getWindow()->GetRenderWindow());
  this->Internals->Interactor->SetInteractorStyle(this->Internals->Style);
  this->Internals->Interactor->Initialize();

  // Disable standard interactor behavior with timer event
  // in order to be able to be able to interact while animating
  this->Internals->Interactor->RemoveObservers(vtkCommand::TimerEvent);
}

//----------------------------------------------------------------------------
void interactor::InitializeAnimation(vtkImporter* importer)
{
  if (!this->Internals->Loader)
  {
    F3DLog::Print(F3DLog::Severity::Error, "Please SetLoader before initializing the animation");
    return;
  }
  this->Internals->AnimationManager.Initialize(
    &this->Internals->Loader->getOptions(), this, this->Internals->Loader->getWindow(), importer);
}

//----------------------------------------------------------------------------
void interactor::setKeyPressCallBack(std::function<bool(int, std::string)> callBack)
{
  this->Internals->KeyPressUserCallBack = callBack;
}

//----------------------------------------------------------------------------
void interactor::setDropFilesCallBack(std::function<bool(std::vector<std::string>)> callBack)
{
  this->Internals->DropFilesUserCallBack = callBack;
}

//----------------------------------------------------------------------------
void interactor::removeTimerCallBack(unsigned long id)
{
  this->Internals->Interactor->RemoveObserver(id);
  this->Internals->Interactor->DestroyTimer(this->Internals->TimerCallBacks[id].first);
}

//----------------------------------------------------------------------------
unsigned long interactor::createTimerCallBack(double time, std::function<void()> callBack)
{
  // Create the timer
  int timerId = this->Internals->Interactor->CreateRepeatingTimer(time);

  // Create the callback and get the observer id
  vtkNew<vtkCallbackCommand> timerCallBack;
  timerCallBack->SetCallback(
    [](vtkObject*, unsigned long, void* clientData, void*)
    {
      std::function<void()>* callBackPtr = static_cast<std::function<void()>*>(clientData);
      (*callBackPtr)();
    });
  unsigned long id =
    this->Internals->Interactor->AddObserver(vtkCommand::TimerEvent, timerCallBack);

  // Store the user callback and set it as client data
  this->Internals->TimerCallBacks[id] = std::make_pair(timerId, callBack);
  timerCallBack->SetClientData(&this->Internals->TimerCallBacks[id].second);
  return id;
}

//----------------------------------------------------------------------------
void interactor::toggleAnimation()
{
  this->Internals->AnimationManager.ToggleAnimation();
}

//----------------------------------------------------------------------------
void interactor::startAnimation()
{
  this->Internals->AnimationManager.StartAnimation();
}

//----------------------------------------------------------------------------
void interactor::stopAnimation()
{
  this->Internals->AnimationManager.StopAnimation();
}

//----------------------------------------------------------------------------
bool interactor::isPlayingAnimation()
{
  return this->Internals->AnimationManager.IsPlaying();
}

//----------------------------------------------------------------------------
void interactor::enableCameraMovement()
{
  this->Internals->Style->SetCameraMovementDisabled(false);
}

//----------------------------------------------------------------------------
void interactor::disableCameraMovement()
{
  this->Internals->Style->SetCameraMovementDisabled(true);
}

//----------------------------------------------------------------------------
bool interactor::playInteraction(const std::string& file)
{
  if (!vtksys::SystemTools::FileExists(file))
  {
    F3DLog::Print(F3DLog::Severity::Error, "Interaction record file to play does not exist ", file);
    return false;
  }
  else
  {
    // Make sure the recorder is off
    this->Internals->Recorder->Off();

    std::string cleanFile = vtksys::SystemTools::CollapseFullPath(file);
    this->Internals->Recorder->SetFileName(cleanFile.c_str());
    this->Internals->Recorder->Play();
  }

  // Recorder can stop the interactor, make sure it is still running
  if (this->Internals->Interactor->GetDone())
  {
    F3DLog::Print(F3DLog::Severity::Error, "Interactor has been stopped");
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
bool interactor::recordInteraction(const std::string& file)
{
  if (file.empty())
  {
    // TODO improve VTK to check file opening
    F3DLog::Print(F3DLog::Severity::Error, "Interaction record file is empty");
    return false;
  }

  // Make sure the recorder is off
  this->Internals->Recorder->Off();

  std::string cleanFile = vtksys::SystemTools::CollapseFullPath(file);
  this->Internals->Recorder->SetFileName(cleanFile.c_str());
  this->Internals->Recorder->On();
  this->Internals->Recorder->Record();

  return true;
}

//----------------------------------------------------------------------------
void interactor::SetInteractorOn(vtkInteractorObserver* observer)
{
  observer->SetInteractor(this->Internals->Interactor);
}

//----------------------------------------------------------------------------
void interactor::start()
{
  this->Internals->Interactor->Start();
}

};
