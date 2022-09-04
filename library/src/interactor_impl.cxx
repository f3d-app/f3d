#include "interactor_impl.h"

#include "animationManager.h"
#include "loader_impl.h"
#include "log.h"
#include "window_impl.h"

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
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

#include <map>

namespace f3d::detail
{
class interactor_impl::internals
{
public:
  internals(options& options, window_impl& window, loader_impl& loader)
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

// Clear needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9229
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 1, 20220601)
    this->Recorder = vtkSmartPointer<vtkF3DInteractorEventRecorder>::New();
    this->Recorder->SetInteractor(this->VTKInteractor);
#endif
  }

  static void OnKeyPress(vtkObject*, unsigned long, void* clientData, void*)
  {
    internals* self = static_cast<internals*>(clientData);
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
    vtkRenderWindow* renWin = self->Window.GetRenderWindow();
    vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
    vtkF3DRendererWithColoring* renWithColor = vtkF3DRendererWithColoring::SafeDownCast(ren);
    bool checkColoring = false;
    bool render = false;

    switch (keyCode)
    {
      case 'C':
        if (renWithColor)
        {
          renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_FIELD_CYCLE);
          self->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
          checkColoring = true;
          render = true;
        }
        break;
      case 'S':
        if (renWithColor)
        {
          renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_ARRAY_CYCLE);
          self->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
          checkColoring = true;
          render = true;
        }
        break;
      case 'Y':
        if (renWithColor)
        {
          renWithColor->CycleScalars(vtkF3DRendererWithColoring::F3D_COMPONENT_CYCLE);
          self->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
          checkColoring = true;
          render = true;
        }
        break;
      case 'B':
        self->Options.toggle("ui.bar");
        render = true;
        break;
      case 'p':
      case 'P':
        self->Options.toggle("render.effect.depth-peeling");
        render = true;
        break;
      case 'Q':
        self->Options.toggle("render.effect.ssao");
        render = true;
        break;
      case 'A':
        self->Options.toggle("render.effect.fxaa");
        render = true;
        break;
      case 'T':
        self->Options.toggle("render.effect.tone-mapping");
        render = true;
        break;
      case 'E':
        self->Options.toggle("render.show-edges");
        render = true;
        break;
      case 'X':
        self->Options.toggle("interactor.axis");
        render = true;
        break;
      case 'G':
        self->Options.toggle("render.grid");
        render = true;
        break;
      case 'N':
        self->Options.toggle("ui.filename");
        render = true;
        break;
      case 'M':
        self->Options.toggle("ui.metadata");
        render = true;
        break;
      case 'Z':
        self->Options.toggle("ui.fps");
        self->Window.render();
        self->Window.render();
        // XXX: Double render is needed here
        break;
      case 'R':
        self->Options.toggle("render.raytracing.enable");
        render = true;
        break;
      case 'D':
        self->Options.toggle("render.raytracing.denoise");
        render = true;
        break;
      case 'V':
        self->Options.toggle("model.volume.enable");
        render = true;
        break;
      case 'I':
        self->Options.toggle("model.volume.inverse");
        render = true;
        break;
      case 'O':
        self->Options.toggle("model.point-sprites.enable");
        render = true;
        break;
      case 'U':
        self->Options.toggle("render.background.blur");
        render = true;
        break;
      case 'K':
        self->Options.toggle("interactor.trackball");
        render = true;
        break;
      case 'H':
        self->Options.toggle("ui.cheatsheet");
        render = true;
        break;
      case '?':
        self->Window.PrintColoringDescription(log::VerboseLevel::INFO);
        self->Window.PrintSceneDescription(log::VerboseLevel::INFO);
        break;
      default:
        if (keySym == "Left")
        {
          self->AnimationManager.StopAnimation();
          loader::LoadFileEnum load = loader::LoadFileEnum::LOAD_PREVIOUS;
          self->Loader.loadFile(load);
          render = true;
        }
        else if (keySym == "Right")
        {
          self->AnimationManager.StopAnimation();
          loader::LoadFileEnum load = loader::LoadFileEnum::LOAD_NEXT;
          self->Loader.loadFile(load);
          render = true;
        }
        else if (keySym == "Up")
        {
          self->AnimationManager.StopAnimation();
          loader::LoadFileEnum load = loader::LoadFileEnum::LOAD_CURRENT;
          self->Loader.loadFile(load);
          render = true;
        }
        else if (keySym == F3D_EXIT_HOTKEY_SYM)
        {
          self->StopInteractor();
        }
        else if (keySym == "Return")
        {
          self->Window.getCamera().resetToDefault();
          render = true;
        }
        else if (keySym == "Space")
        {
          self->AnimationManager.ToggleAnimation();
        }
        break;
    }

    if (checkColoring)
    {
      // Resynchronise renderer coloring status with options
      self->Options.set("model.scivis.cells", renWithColor->GetColoringUseCell());
      self->Options.set("model.scivis.array-name", renWithColor->GetColoringArrayName());
      self->Options.set("model.scivis.component", renWithColor->GetColoringComponent());
    }
    if (render)
    {
      self->Window.render();
    }
  }

  static void OnDropFiles(vtkObject*, unsigned long, void* clientData, void* callData)
  {
    internals* self = static_cast<internals*>(clientData);
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
    self->Loader.loadFile(loader::LoadFileEnum::LOAD_LAST);
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

  options& Options;
  window_impl& Window;
  loader_impl& Loader;
  animationManager AnimationManager;

  vtkNew<vtkRenderWindowInteractor> VTKInteractor;
  vtkNew<vtkF3DInteractorStyle> Style;
  vtkSmartPointer<vtkF3DInteractorEventRecorder> Recorder;
  int WindowSize[2] = { -1, -1 };
  int WindowPos[2] = { 0, 0 };
  std::map<unsigned long, std::pair<int, std::function<void()> > > TimerCallBacks;
};

//----------------------------------------------------------------------------
interactor_impl::interactor_impl(options& options, window_impl& window, loader_impl& loader)
  : Internals(std::make_unique<interactor_impl::internals>(options, window, loader))
{
  // Loader need the interactor
  this->Internals->Loader.setInteractor(this);
}

//----------------------------------------------------------------------------
interactor_impl::~interactor_impl() = default;

//----------------------------------------------------------------------------
interactor& interactor_impl::setKeyPressCallBack(std::function<bool(int, std::string)> callBack)
{
  this->Internals->KeyPressUserCallBack = callBack;
  return *this;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::setDropFilesCallBack(
  std::function<bool(std::vector<std::string>)> callBack)
{
  this->Internals->DropFilesUserCallBack = callBack;
  return *this;
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
    log::error("Interaction record file to play does not exist ", file);
    return false;
  }
  else
  {
// Clear needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9229
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 1, 20220601)
    // Make sure the recorder is off and streams are cleared
    this->Internals->Recorder->Off();
    this->Internals->Recorder->Clear();
#else
    // Create a clean recorder as its internal state matters
    this->Internals->Recorder = vtkSmartPointer<vtkF3DInteractorEventRecorder>::New();
    this->Internals->Recorder->SetInteractor(this->Internals->VTKInteractor);
#endif

    std::string cleanFile = vtksys::SystemTools::CollapseFullPath(file);
    this->Internals->Recorder->SetFileName(cleanFile.c_str());
    this->Internals->Window.UpdateDynamicOptions();
    this->Internals->Recorder->Play();
  }

  // Recorder can stop the interactor, make sure it is still running
  if (this->Internals->VTKInteractor->GetDone())
  {
    log::error("Interactor has been stopped");
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
    log::error("No interaction record file provided");
    return false;
  }

// Clear needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9229
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 1, 20220601)
  // Make sure the recorder is off and streams are cleared
  this->Internals->Recorder->Off();
  this->Internals->Recorder->Clear();
#else
  // Create a clean recorder as its internal state matters
  this->Internals->Recorder = vtkSmartPointer<vtkF3DInteractorEventRecorder>::New();
  this->Internals->Recorder->SetInteractor(this->Internals->VTKInteractor);
#endif

  std::string cleanFile = vtksys::SystemTools::CollapseFullPath(file);
  this->Internals->Recorder->SetFileName(cleanFile.c_str());
  this->Internals->Recorder->On();
  this->Internals->Recorder->Record();

  return true;
}

//----------------------------------------------------------------------------
void interactor_impl::start()
{
  this->Internals->Window.UpdateDynamicOptions();
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
}
