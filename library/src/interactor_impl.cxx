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
#include <vtkCellPicker.h>
#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkNew.h>
#include <vtkPicker.h>
#include <vtkPointPicker.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkStringArray.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

#include <chrono>
#include <cmath>
#include <map>

#include "camera.h"

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
#ifdef __EMSCRIPTEN__
    vtkRenderWindowInteractor::InteractorManagesTheEventLoop = false;
#endif
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

    vtkNew<vtkCallbackCommand> middleButtonPressCallback;
    middleButtonPressCallback->SetClientData(this);
    middleButtonPressCallback->SetCallback(OnMiddleButtonPress);
    this->Style->AddObserver(vtkCommand::MiddleButtonPressEvent, middleButtonPressCallback);

    vtkNew<vtkCallbackCommand> middleButtonReleaseCallback;
    middleButtonReleaseCallback->SetClientData(this);
    middleButtonReleaseCallback->SetCallback(OnMiddleButtonRelease);
    this->Style->AddObserver(vtkCommand::MiddleButtonReleaseEvent, middleButtonReleaseCallback);

    this->Recorder = vtkSmartPointer<vtkF3DInteractorEventRecorder>::New();
    this->Recorder->SetInteractor(this->VTKInteractor);
  }

  //----------------------------------------------------------------------------
  // Method defined to normalize the Z axis so all models are treated temporarily
  // as Z-up axis models.
  void ToEnvironmentSpace(vtkMatrix3x3* transform)
  {
    vtkRenderer* renderer =
      this->VTKInteractor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    const double* up = renderer->GetEnvironmentUp();
    const double* right = renderer->GetEnvironmentRight();
    double fwd[3];
    vtkMath::Cross(right, up, fwd);
    const double m[9] = {
      right[0], right[1], right[2], //
      fwd[0], fwd[1], fwd[2],       //
      up[0], up[1], up[2],          //
    };
    transform->DeepCopy(m);
  }

  //----------------------------------------------------------------------------
  // Set the view orbit position on the viewport.
  enum class ViewType
  {
    VT_FRONT,
    VT_RIGHT,
    VT_TOP,
    VT_ISOMETRIC
  };
  static void SetViewOrbit(ViewType view, internals* self)
  {
    vtkNew<vtkMatrix3x3> transform;
    self->ToEnvironmentSpace(transform);
    camera& cam = self->Window.getCamera();
    vector3_t up = { 0, 0, 1 };
    point3_t foc = cam.getFocalPoint();
    point3_t axis, newPos;

    switch (view)
    {
      case ViewType::VT_FRONT:
        axis = { 0, +1, 0 };
        break;
      case ViewType::VT_RIGHT:
        axis = { +1, 0, 0 };
        break;
      case ViewType::VT_TOP:
        axis = { 0, 0, +1 };
        up = { 0, -1, 0 };
        break;
      case ViewType::VT_ISOMETRIC:
        axis = { -1, +1, +1 };
        break;
    }

    transform->MultiplyPoint(up.data(), up.data());
    transform->MultiplyPoint(axis.data(), axis.data());

    newPos[0] = foc[0] + axis[0];
    newPos[1] = foc[1] + axis[1];
    newPos[2] = foc[2] + axis[2];

    /* set camera coordinates back */
    cam.setPosition(newPos);
    cam.setViewUp(up);
    cam.resetToBounds(0.9);
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

    // Available keycodes: None
    switch (keyCode)
    {
      case 'W':
        self->AnimationManager->CycleAnimation();
        self->Options.scene.animation.index = self->AnimationManager->GetAnimationIndex();
        ren->SetAnimationnameInfo(self->AnimationManager->GetAnimationName());
        render = true;
        break;
      case 'C':
        if (renWithColor)
        {
          renWithColor->CycleScalars(vtkF3DRendererWithColoring::CycleType::FIELD);
          self->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
          checkColoring = true;
          render = true;
        }
        break;
      case 'S':
        if (renWithColor)
        {
          renWithColor->CycleScalars(vtkF3DRendererWithColoring::CycleType::ARRAY_INDEX);
          self->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
          checkColoring = true;
          render = true;
        }
        break;
      case 'Y':
        if (renWithColor)
        {
          renWithColor->CycleScalars(vtkF3DRendererWithColoring::CycleType::COMPONENT);
          self->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
          checkColoring = true;
          render = true;
        }
        break;
      case 'B':
        self->Options.ui.scalar_bar = !self->Options.ui.scalar_bar;
        render = true;
        break;
      case 'P':
        self->Options.render.effect.translucency_support =
          !self->Options.render.effect.translucency_support;
        render = true;
        break;
      case 'Q':
        self->Options.render.effect.ambient_occlusion =
          !self->Options.render.effect.ambient_occlusion;
        render = true;
        break;
      case 'A':
        self->Options.render.effect.anti_aliasing = !self->Options.render.effect.anti_aliasing;
        render = true;
        break;
      case 'T':
        self->Options.render.effect.tone_mapping = !self->Options.render.effect.tone_mapping;
        render = true;
        break;
      case 'E':
        self->Options.render.show_edges = !self->Options.render.show_edges;
        render = true;
        break;
      case 'X':
        self->Options.interactor.axis = !self->Options.interactor.axis;
        render = true;
        break;
      case 'G':
        self->Options.render.grid.enable = !self->Options.render.grid.enable;
        render = true;
        break;
      case 'N':
        self->Options.ui.filename = !self->Options.ui.filename;
        render = true;
        break;
      case 'M':
        self->Options.ui.metadata = !self->Options.ui.metadata;
        render = true;
        break;
      case 'Z':
        self->Options.ui.fps = !self->Options.ui.fps;
        self->Window.render();
        self->Window.render();
        // XXX: Double render is needed here
        break;
      case 'R':
        self->Options.render.raytracing.enable = !self->Options.render.raytracing.enable;
        render = true;
        break;
      case 'D':
        self->Options.render.raytracing.denoise = !self->Options.render.raytracing.denoise;
        render = true;
        break;
      case 'V':
        self->Options.model.volume.enable = !self->Options.model.volume.enable;
        render = true;
        break;
      case 'I':
        self->Options.model.volume.inverse = !self->Options.model.volume.inverse;
        render = true;
        break;
      case 'O':
        self->Options.model.point_sprites.enable = !self->Options.model.point_sprites.enable;
        render = true;
        break;
      case 'U':
        self->Options.render.background.blur = !self->Options.render.background.blur;
        render = true;
        break;
      case 'K':
        self->Options.interactor.trackball = !self->Options.interactor.trackball;
        render = true;
        break;
      case 'F':
        self->Options.render.hdri.ambient = !self->Options.render.hdri.ambient;
        render = true;
        break;
      case 'J':
        self->Options.render.background.skybox = !self->Options.render.background.skybox;
        render = true;
        break;
      case 'L':
      {
        const double intensity = self->Options.render.light.intensity;
        const bool down = rwi->GetShiftKey();

        /* `ref < x` is equivalent to:
         * - `intensity <= x` when going down
         * - `intensity < x` when going up */
        const double ref = down ? intensity - 1e-6 : intensity;
        // clang-format off
        /* offset in percentage points */
        const int offsetPp = ref < .5 ?  1
                           : ref <  1 ?  2
                           : ref <  5 ?  5
                           : ref < 10 ? 10
                           :            25;
        // clang-format on
        /* new intensity in percents */
        const int newIntensityPct = std::lround(intensity * 100) + (down ? -offsetPp : +offsetPp);

        self->Options.render.light.intensity = std::max(newIntensityPct, 0) / 100.0;
        render = true;
        break;
      }
      case 'H':
        self->Options.ui.cheatsheet = !self->Options.ui.cheatsheet;
        render = true;
        break;
      case '?':
        self->Window.PrintColoringDescription(log::VerboseLevel::INFO);
        self->Window.PrintSceneDescription(log::VerboseLevel::INFO);
        break;
      case '1':
        self->SetViewOrbit(ViewType::VT_FRONT, self);
        render = true;
        break;
      case '3':
        self->SetViewOrbit(ViewType::VT_RIGHT, self);
        render = true;
        break;
      case '5':
        self->Options.scene.camera.orthographic = !self->Options.scene.camera.orthographic;
        render = true;
        break;
      case '7':
        self->SetViewOrbit(ViewType::VT_TOP, self);
        render = true;
        break;
      case '9':
        self->SetViewOrbit(ViewType::VT_ISOMETRIC, self);
        render = true;
        break;
      default:
        if (keySym == F3D_EXIT_HOTKEY_SYM)
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
          assert(self->AnimationManager);
          self->AnimationManager->ToggleAnimation();
        }
        break;
    }

    if (checkColoring)
    {
      // Resynchronise renderer coloring status with options
      self->Options.model.scivis.enable = renWithColor->GetColoringEnabled();
      self->Options.model.scivis.cells = renWithColor->GetColoringUseCell();
      self->Options.model.scivis.array_name = renWithColor->GetColoringArrayName();
      self->Options.model.scivis.component = renWithColor->GetColoringComponent();
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
    filesVec.resize(filesArr->GetNumberOfTuples());
    for (int i = 0; i < filesArr->GetNumberOfTuples(); i++)
    {
      filesVec[i] = filesArr->GetValue(i);
    }

    if (self->DropFilesUserCallBack(filesVec))
    {
      return;
    }

    // No user defined behavior, load the first file
    if (!filesVec.empty())
    {
      assert(self->AnimationManager);
      self->AnimationManager->StopAnimation();
      if (self->Loader.hasSceneReader(filesVec[0]))
      {
        self->Loader.loadScene(filesVec[0]);
      }
      else if (self->Loader.hasGeometryReader(filesVec[0]))
      {
        self->Loader.loadGeometry(filesVec[0], true);
      }
      self->Window.render();
    }
  }

  static void OnMiddleButtonPress(vtkObject*, unsigned long, void* clientData, void*)
  {
    internals* self = static_cast<internals*>(clientData);

    self->VTKInteractor->GetEventPosition(self->MiddleButtonDownPosition);

    self->Style->OnMiddleButtonDown();
  }

  static void OnMiddleButtonRelease(vtkObject*, unsigned long, void* clientData, void*)
  {
    internals* self = static_cast<internals*>(clientData);

    const int* middleButtonUpPosition = self->VTKInteractor->GetEventPosition();

    const int xDelta = middleButtonUpPosition[0] - self->MiddleButtonDownPosition[0];
    const int yDelta = middleButtonUpPosition[1] - self->MiddleButtonDownPosition[1];
    const int sqPosDelta = xDelta * xDelta + yDelta * yDelta;
    if (sqPosDelta < self->DragDistanceTol * self->DragDistanceTol)
    {
      const int x = self->MiddleButtonDownPosition[0];
      const int y = self->MiddleButtonDownPosition[1];
      vtkRenderer* renderer =
        self->VTKInteractor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();

      bool pickSuccessful = false;
      double picked[3];
      if (self->CellPicker->Pick(x, y, 0, renderer))
      {
        self->CellPicker->GetPickPosition(picked);
        pickSuccessful = true;
      }
      else if (self->PointPicker->Pick(x, y, 0, renderer))
      {
        self->PointPicker->GetPickPosition(picked);
        pickSuccessful = true;
      }

      if (pickSuccessful)
      {
        /*     pos.--------------------.foc
         *       /|                   /
         *      / |                  /
         *     .--.-----------------.picked
         * pos1    pos2
         */
        const camera_state_t state = self->Window.getCamera().getState();

        double focV[3];
        vtkMath::Subtract(picked, state.foc.data(), focV); /* foc -> picked */

        double posV[3];
        vtkMath::Subtract(picked, state.foc.data(), posV); /* pos -> pos1, parallel to focV */
        if (!self->Style->GetInteractor()->GetShiftKey())
        {
          double v[3];
          vtkMath::Subtract(state.foc.data(), state.pos.data(), v); /* pos -> foc */
          vtkMath::ProjectVector(focV, v, v);                       /* pos2 -> pos1 */
          vtkMath::Subtract(posV, v, posV); /* pos -> pos2, keeps on camera plane */
        }

        const auto interpolateCameraState = [&state, &focV, &posV](double ratio) -> camera_state_t
        {
          return { //
            {
              state.pos[0] + posV[0] * ratio,
              state.pos[1] + posV[1] * ratio,
              state.pos[2] + posV[2] * ratio,
            },
            {
              state.foc[0] + focV[0] * ratio,
              state.foc[1] + focV[1] * ratio,
              state.foc[2] + focV[2] * ratio,
            },
            state.up, state.angle
          };
        };

        self->AnimateCameraTransition(interpolateCameraState);
      }
    }

    self->Style->OnMiddleButtonUp();
  }

  std::function<bool(int, const std::string&)> KeyPressUserCallBack = [](int, const std::string&)
  { return false; };
  std::function<bool(const std::vector<std::string>&)> DropFilesUserCallBack =
    [](const std::vector<std::string>&) { return false; };

  void StartInteractor()
  {
    this->VTKInteractor->Start();
  }

  void StopInteractor()
  {
    this->VTKInteractor->RemoveObservers(vtkCommand::TimerEvent);
    this->VTKInteractor->ExitCallback();
  }

  /**
   * Run a camera transition animation based on a camera state interpolation function.
   * The provided function will be called with an interpolation parameter
   * varying from `0.` for the initial state to `1.` for the final state;
   * it shall return an appropriate linearly interpolated `camera_state_t` for any value in between.
   */
  template<class CameraStateInterpolator>
  void AnimateCameraTransition(CameraStateInterpolator interpolateCameraState)
  {
    window& win = this->Window;
    camera& cam = win.getCamera();
    const int duration = this->TransitionDuration;

    if (duration > 0)
    {
      // TODO implement a way to not queue key presses while the animation is running

      const auto start = std::chrono::high_resolution_clock::now();
      const auto end = start + std::chrono::milliseconds(duration);
      auto now = start;
      while (now < end)
      {
        const double timeDelta =
          std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        const double ratio = (1 - std::cos(vtkMath::Pi() * (timeDelta / duration))) / 2;
        cam.setState(interpolateCameraState(ratio));
        this->Window.render();
        now = std::chrono::high_resolution_clock::now();
      }
    }

    cam.setState(interpolateCameraState(1.)); // ensure final update
    this->Window.render();
  }

  options& Options;
  window_impl& Window;
  loader_impl& Loader;
  animationManager* AnimationManager;

  vtkNew<vtkRenderWindowInteractor> VTKInteractor;
  vtkNew<vtkF3DInteractorStyle> Style;
  vtkSmartPointer<vtkF3DInteractorEventRecorder> Recorder;
  std::map<unsigned long, std::pair<int, std::function<void()>>> TimerCallBacks;

  vtkNew<vtkCellPicker> CellPicker;
  vtkNew<vtkPointPicker> PointPicker;

  int MiddleButtonDownPosition[2] = { 0, 0 };

  int DragDistanceTol = 3;      /* px */
  int TransitionDuration = 100; /* ms */
};

//----------------------------------------------------------------------------
interactor_impl::interactor_impl(options& options, window_impl& window, loader_impl& loader)
  : Internals(std::make_unique<interactor_impl::internals>(options, window, loader))
{
  // Loader need the interactor, loader will set the AnimationManager on the interactor
  this->Internals->Loader.SetInteractor(this);
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
  assert(this->Internals->AnimationManager);
  this->Internals->AnimationManager->ToggleAnimation();
}

//----------------------------------------------------------------------------
void interactor_impl::startAnimation()
{
  assert(this->Internals->AnimationManager);
  this->Internals->AnimationManager->StartAnimation();
}

//----------------------------------------------------------------------------
void interactor_impl::stopAnimation()
{
  assert(this->Internals->AnimationManager);
  this->Internals->AnimationManager->StopAnimation();
}

//----------------------------------------------------------------------------
bool interactor_impl::isPlayingAnimation()
{
  assert(this->Internals->AnimationManager);
  return this->Internals->AnimationManager->IsPlaying();
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
    // Make sure the recorder is off and streams are cleared
    this->Internals->Recorder->Off();
    this->Internals->Recorder->Clear();

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
    log::error("No interaction record file provided");
    return false;
  }

  std::string cleanFile = vtksys::SystemTools::CollapseFullPath(file);

  std::string parentDirectory = vtksys::SystemTools::GetParentDirectory(cleanFile);

  // Check if the parent directory exists
  if (!vtksys::SystemTools::FileExists(parentDirectory))
  {
    log::error("Interaction record directory does not exist ", parentDirectory);
    return false;
  }

  // Check if we can write to the directory
  if (!vtksys::SystemTools::TestFileAccess(parentDirectory, vtksys::TEST_FILE_WRITE))
  {
    log::error("Don't have write permissions for ", parentDirectory);
    return false;
  }

  // Make sure the recorder is off and streams are cleared
  this->Internals->Recorder->Off();
  this->Internals->Recorder->Clear();

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
void interactor_impl::SetAnimationManager(animationManager* manager)
{
  this->Internals->AnimationManager = manager;
}

//----------------------------------------------------------------------------
void interactor_impl::SetInteractorOn(vtkInteractorObserver* observer)
{
  observer->SetInteractor(this->Internals->VTKInteractor);
}

//----------------------------------------------------------------------------
void interactor_impl::UpdateRendererAfterInteraction()
{
  this->Internals->Style->UpdateRendererAfterInteraction();
}
}
