#include "f3d_interactor_impl.h"

#include "F3DAnimationManager.h"
#include "f3d_loader_impl.h"
#include "f3d_log.h"
#include "f3d_options.h"
#include "f3d_window_impl.h"

#include "vtkF3DConfigure.h"
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
class interactor_impl::F3DInternals
{
public:
  F3DInternals(const options& options, window_impl& window, loader_impl& loader)
    : Options(options)
    , Window(window)
    , Loader(loader)
  {
    this->VTKInteractor->SetRenderWindow(this->Window.GetRenderWindow());
    this->VTKInteractor->SetInteractorStyle(this->Style);
    this->VTKInteractor->Initialize();

    // Disable standard interactor behavior with timer event
    // in order to be able to interact while animating
    this->VTKInteractor->RemoveObservers(vtkCommand::TimerEvent);

    vtkNew<vtkCallbackCommand> keyPressCallback;
    keyPressCallback->SetClientData(this);
    keyPressCallback->SetCallback(OnKeyPress);
    this->Style->AddObserver(vtkF3DInteractorStyle::KeyPressEvent, keyPressCallback);

    vtkNew<vtkCallbackCommand> dropFilesCallback;
    dropFilesCallback->SetClientData(this);
    dropFilesCallback->SetCallback(OnDropFiles);
    this->Style->AddObserver(vtkF3DInteractorStyle::DropFilesEvent, dropFilesCallback);

    this->Recorder->SetInteractor(this->VTKInteractor);
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
          if (self->Options.getAsBool("verbose"))
          {
            f3d::log::info(renWithColor->GetRenderingDescription());
          }
          renWin->Render();
        }
        break;
      case 'S':
        if (renWithColor)
        {
          renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_ARRAY_CYCLE);
          if (self->Options.getAsBool("verbose"))
          {
            f3d::log::info(renWithColor->GetRenderingDescription());
          }
          renWin->Render();
        }
        break;
      case 'Y':
        if (renWithColor)
        {
          renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_COMPONENT_CYCLE);
          if (self->Options.getAsBool("verbose"))
          {
            f3d::log::info(renWithColor->GetRenderingDescription());
          }
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
          if (self->Options.getAsBool("verbose"))
          {
            f3d::log::info(renWithColor->GetRenderingDescription());
          }
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
          if (self->Options.getAsBool("verbose"))
          {
            f3d::log::info(renWithColor->GetRenderingDescription());
          }
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
      {
        std::string output = ren->GetSceneDescription();
        f3d::log::info(output);
      }
      break;
      default:
        if (keySym == "Left")
        {
          self->AnimationManager.StopAnimation();
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_PREVIOUS;
          self->Loader.loadFile(load);
          renWin->Render();
        }
        else if (keySym == "Right")
        {
          self->AnimationManager.StopAnimation();
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_NEXT;
          self->Loader.loadFile(load);
          renWin->Render();
        }
        else if (keySym == "Up")
        {
          self->AnimationManager.StopAnimation();
          f3d::loader::LoadFileEnum load = f3d::loader::LoadFileEnum::LOAD_CURRENT;
          self->Loader.loadFile(load);
          renWin->Render();
        }
        else if (keySym == F3D_EXIT_HOTKEY_SYM)
        {
          self->StopInteractor();
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
      self->Loader.addFile(file);
    }
    self->Loader.loadFile(f3d::loader::LoadFileEnum::LOAD_LAST);
    self->Window.render();
  }

  std::function<bool(int, std::string)> KeyPressUserCallBack = [](int, std::string)
  { return false; };
  std::function<bool(std::vector<std::string>)> DropFilesUserCallBack = [](std::vector<std::string>)
  { return false; };

  void StartInteractor() { this->VTKInteractor->Start(); }

  void StopInteractor()
  {
    this->VTKInteractor->RemoveObservers(vtkCommand::TimerEvent);
    this->VTKInteractor->ExitCallback();
  }

  const f3d::options& Options;
  f3d::window_impl& Window;
  f3d::loader_impl& Loader;

  F3DAnimationManager AnimationManager;
  vtkNew<vtkRenderWindowInteractor> VTKInteractor;
  vtkNew<vtkF3DInteractorStyle> Style;
  vtkNew<vtkF3DInteractorEventRecorder> Recorder;
  int WindowSize[2] = { -1, -1 };
  int WindowPos[2] = { 0, 0 };
  std::map<unsigned long, std::pair<int, std::function<void()> > > TimerCallBacks;
};

//----------------------------------------------------------------------------
interactor_impl::interactor_impl(const options& options, window_impl& window, loader_impl& loader)
  : Internals(new interactor_impl::F3DInternals(options, window, loader))
{
  // Loader need the interactor
  this->Internals->Loader.setInteractor(this);
}

//----------------------------------------------------------------------------
interactor_impl::~interactor_impl() = default;

//----------------------------------------------------------------------------
void interactor_impl::setKeyPressCallBack(std::function<bool(int, std::string)> callBack)
{
  this->Internals->KeyPressUserCallBack = callBack;
}

//----------------------------------------------------------------------------
void interactor_impl::setDropFilesCallBack(std::function<bool(std::vector<std::string>)> callBack)
{
  this->Internals->DropFilesUserCallBack = callBack;
}

//----------------------------------------------------------------------------
void interactor_impl::removeTimerCallBack(unsigned long id)
{
  this->Internals->VTKInteractor->RemoveObserver(id);
  this->Internals->VTKInteractor->DestroyTimer(this->Internals->TimerCallBacks[id].first);
}

//----------------------------------------------------------------------------
unsigned long interactor_impl::createTimerCallBack(double time, std::function<void()> callBack)
{
  // Create the timer
  int timerId = this->Internals->VTKInteractor->CreateRepeatingTimer(time);

  // Create the callback and get the observer id
  vtkNew<vtkCallbackCommand> timerCallBack;
  timerCallBack->SetCallback(
    [](vtkObject*, unsigned long, void* clientData, void*)
    {
      std::function<void()>* callBackPtr = static_cast<std::function<void()>*>(clientData);
      (*callBackPtr)();
    });
  unsigned long id =
    this->Internals->VTKInteractor->AddObserver(vtkCommand::TimerEvent, timerCallBack);

  // Store the user callback and set it as client data
  this->Internals->TimerCallBacks[id] = std::make_pair(timerId, callBack);
  timerCallBack->SetClientData(&this->Internals->TimerCallBacks[id].second);
  return id;
}

//----------------------------------------------------------------------------
void interactor_impl::toggleAnimation()
{
  this->Internals->AnimationManager.ToggleAnimation();
}

//----------------------------------------------------------------------------
void interactor_impl::startAnimation()
{
  this->Internals->AnimationManager.StartAnimation();
}

//----------------------------------------------------------------------------
void interactor_impl::stopAnimation()
{
  this->Internals->AnimationManager.StopAnimation();
}

//----------------------------------------------------------------------------
bool interactor_impl::isPlayingAnimation()
{
  return this->Internals->AnimationManager.IsPlaying();
}

//----------------------------------------------------------------------------
void interactor_impl::enableCameraMovement()
{
  this->Internals->Style->SetCameraMovementDisabled(false);
}

//----------------------------------------------------------------------------
void interactor_impl::disableCameraMovement()
{
  this->Internals->Style->SetCameraMovementDisabled(true);
}

//----------------------------------------------------------------------------
bool interactor_impl::playInteraction(const std::string& file)
{
  if (!vtksys::SystemTools::FileExists(file))
  {
    f3d::log::error("Interaction record file to play does not exist ", file);
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
  if (this->Internals->VTKInteractor->GetDone())
  {
    f3d::log::error("Interactor has been stopped");
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
bool interactor_impl::recordInteraction(const std::string& file)
{
  if (file.empty())
  {
    // TODO improve VTK to check file opening
    f3d::log::error("Interaction record file is empty");
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
void interactor_impl::start()
{
  this->Internals->StartInteractor();
}

//----------------------------------------------------------------------------
void interactor_impl::stop()
{
  this->Internals->StopInteractor();
}

//----------------------------------------------------------------------------
void interactor_impl::SetInteractorOn(vtkInteractorObserver* observer)
{
  observer->SetInteractor(this->Internals->VTKInteractor);
}

//----------------------------------------------------------------------------
void interactor_impl::InitializeAnimation(vtkImporter* importer)
{
  this->Internals->AnimationManager.Initialize(
    &this->Internals->Options, this, &this->Internals->Window, importer);
}
};
