#include "interactor_impl.h"

#include "animationManager.h"
#include "log.h"
#include "scene_impl.h"
#include "utils.h"
#include "window_impl.h"

#include "vtkF3DConsoleOutputWindow.h"

#if F3D_MODULE_UI
#include "vtkF3DImguiConsole.h"
#endif

#include "vtkF3DInteractorEventRecorder.h"
#include "vtkF3DInteractorStyle.h"
#include "vtkF3DRenderer.h"
#include "vtkF3DUIActor.h"
#include "vtkF3DUIObserver.h"

#include <vtkCallbackCommand.h>
#include <vtkCellPicker.h>
#include <vtkGenericRenderWindowInteractor.h>
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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <regex>
#include <vector>

#include "camera.h"

namespace fs = std::filesystem;

namespace f3d::detail
{
using mod_t = interaction_bind_t::ModifierKeys;

class interactor_impl::internals
{
public:
  struct BindingCommands
  {
    std::vector<std::string> CommandVector;
    documentation_callback_t DocumentationCallback;
  };

  internals(options& options, window_impl& window, scene_impl& scene, interactor_impl& inter)
    : Options(options)
    , Window(window)
    , Scene(scene)
    , Interactor(inter)
  {
    window::Type type = window.getType();
    if (type == window::Type::GLX || type == window::Type::WGL || type == window::Type::COCOA ||
      type == window::Type::WASM)
    {
      this->VTKInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    }
    else
    {
      this->VTKInteractor = vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();
    }

#ifdef __EMSCRIPTEN__
    vtkRenderWindowInteractor::InteractorManagesTheEventLoop = false;
#endif
    this->VTKInteractor->SetRenderWindow(this->Window.GetRenderWindow());
    this->VTKInteractor->SetInteractorStyle(this->Style);
    this->VTKInteractor->Initialize();

    // Some implementation (e.g. macOS) in VTK set the window name during initialization
    // so we need to set the name right after initialization
    this->Window.setWindowName("f3d");

    this->UIObserver->InstallObservers(this->VTKInteractor);

    // observe console event to trigger commands
    vtkNew<vtkCallbackCommand> commandCallback;
    commandCallback->SetClientData(this);
    commandCallback->SetCallback(OnConsoleEvent);
    vtkOutputWindow::GetInstance()->AddObserver(
      vtkF3DConsoleOutputWindow::TriggerEvent, commandCallback);
    vtkOutputWindow::GetInstance()->AddObserver(
      vtkF3DConsoleOutputWindow::ShowEvent, commandCallback);
    vtkOutputWindow::GetInstance()->AddObserver(
      vtkF3DConsoleOutputWindow::HideEvent, commandCallback);

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

    this->Style->ResetTemporaryUp();
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

  void SetViewOrbit(ViewType view)
  {
    vtkNew<vtkMatrix3x3> transform;
    this->ToEnvironmentSpace(transform);
    camera& cam = this->Window.getCamera();
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

  //----------------------------------------------------------------------------
  // Increase/Decrease light intensity
  void IncreaseLightIntensity(bool negative)
  {
    const double intensity = this->Options.render.light.intensity;

    /* `ref < x` is equivalent to:
     * - `intensity <= x` when going down
     * - `intensity < x` when going up */
    const double ref = negative ? intensity - 1e-6 : intensity;
    // clang-format off
      /* offset in percentage points */
      const int offsetPp = ref < .5 ?  1
      : ref <  1 ?  2
      : ref <  5 ?  5
      : ref < 10 ? 10
      :            25;
    // clang-format on

    /* new intensity in percents */
    const int newIntensityPct = std::lround(intensity * 100) + (negative ? -offsetPp : +offsetPp);
    this->Options.render.light.intensity = std::max(newIntensityPct, 0) / 100.0;
  }

  //----------------------------------------------------------------------------
  // Increase/Decrease opacity
  void IncreaseOpacity(bool negative)
  {
    // current opacity, interpreted as 1 if it does not exist
    const double currentOpacity = this->Options.model.color.opacity.value_or(1.0);

    // new opacity, clamped between 0 and 1 if not already set outside that range
    const double increment = negative ? -0.05 : 0.05;
    double newOpacity = currentOpacity + increment;
    if (currentOpacity <= 1.0 && 0.0 <= currentOpacity)
    {
      newOpacity = std::min(1.0, std::max(0.0, newOpacity));
    }

    this->Options.model.color.opacity = newOpacity;
  }

  //----------------------------------------------------------------------------
  // Synchronise options from the renderer properties
  static void SynchronizeScivisOptions(f3d::options& opt, vtkF3DRenderer* ren)
  {
    // Synchronize renderer coloring status with scivis options
    opt.model.scivis.enable = ren->GetEnableColoring();
    opt.model.scivis.cells = ren->GetUseCellColoring();
    opt.model.scivis.array_name = ren->GetArrayNameForColoring();
    opt.model.scivis.component = ren->GetComponentForColoring();
  }

  //----------------------------------------------------------------------------
  static void OnConsoleEvent(vtkObject*, unsigned long event, void* clientData, void* data)
  {
    internals* self = static_cast<internals*>(clientData);

    if (event == vtkF3DConsoleOutputWindow::TriggerEvent)
    {
      const char* commandWithArgs = static_cast<const char*>(data);
      self->Interactor.SetCommandBuffer(commandWithArgs);
    }
    else if (event == vtkF3DConsoleOutputWindow::ShowEvent ||
      event == vtkF3DConsoleOutputWindow::HideEvent)
    {
      self->Options.ui.console = (event == vtkF3DConsoleOutputWindow::ShowEvent);
    }

    self->RenderRequested = true;
  }

  //----------------------------------------------------------------------------
  static void OnKeyPress(vtkObject*, unsigned long, void* clientData, void*)
  {
    internals* self = static_cast<internals*>(clientData);
    vtkRenderWindowInteractor* rwi = self->Style->GetInteractor();
    std::string interaction = rwi->GetKeySym();
    if (!interaction.empty())
    {
      // Make sure key symbols starts with an upper char (e.g. "space" -> "Space")
      interaction[0] = std::toupper(interaction[0]);
    }

    self->TriggerBinding(interaction, "");
  }

  //----------------------------------------------------------------------------
  static void OnDropFiles(vtkObject*, unsigned long, void* clientData, void* callData)
  {
    internals* self = static_cast<internals*>(clientData);
    vtkStringArray* filesArr = static_cast<vtkStringArray*>(callData);
    const std::regex charsToEscape(R"((["\\]))");
    std::string filesString;
    for (int i = 0; i < filesArr->GetNumberOfTuples(); i++)
    {
      const vtkStdString& filename = filesArr->GetValue(i);
      const std::string escapedFilename = std::regex_replace(filename, charsToEscape, "\\$1");
      if (i > 0)
      {
        filesString.push_back(' ');
      }
      filesString.push_back('"');
      filesString.append(escapedFilename);
      filesString.push_back('"');
    }

    self->TriggerBinding("Drop", filesString);
  }

  //----------------------------------------------------------------------------
  static void OnMiddleButtonPress(vtkObject*, unsigned long, void* clientData, void*)
  {
    internals* self = static_cast<internals*>(clientData);

    self->VTKInteractor->GetEventPosition(self->MiddleButtonDownPosition);

    self->Style->OnMiddleButtonDown();
  }

  //----------------------------------------------------------------------------
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
        vtkMath::Subtract(picked, state.focalPoint.data(), focV); /* foc -> picked */

        double posV[3];
        vtkMath::Subtract(
          picked, state.focalPoint.data(), posV); /* pos -> pos1, parallel to focV */
        if (!self->Style->GetInteractor()->GetShiftKey())
        {
          double v[3];
          vtkMath::Subtract(state.focalPoint.data(), state.position.data(), v); /* pos -> foc */
          vtkMath::ProjectVector(focV, v, v);                                   /* pos2 -> pos1 */
          vtkMath::Subtract(posV, v, posV); /* pos -> pos2, keeps on camera plane */
        }

        const auto interpolateCameraState = [&state, &focV, &posV](double ratio) -> camera_state_t
        {
          return { {
                     state.position[0] + posV[0] * ratio,
                     state.position[1] + posV[1] * ratio,
                     state.position[2] + posV[2] * ratio,
                   },
            {
              state.focalPoint[0] + focV[0] * ratio,
              state.focalPoint[1] + focV[1] * ratio,
              state.focalPoint[2] + focV[2] * ratio,
            },
            state.viewUp, state.viewAngle };
        };

        self->AnimateCameraTransition(interpolateCameraState);
      }
    }

    self->Style->OnMiddleButtonUp();
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

  //----------------------------------------------------------------------------
  void TriggerBinding(const std::string& interaction, const std::string& argsString)
  {
    mod_t mod = mod_t::NONE;
    vtkRenderWindowInteractor* rwi = this->Style->GetInteractor();
    const bool shift = rwi->GetShiftKey() == 1;
    const bool ctrl = rwi->GetControlKey() == 1;
    if (shift && ctrl)
    {
      mod = mod_t::CTRL_SHIFT;
    }
    else if (ctrl)
    {
      mod = mod_t::CTRL;
    }
    else if (shift)
    {
      mod = mod_t::SHIFT;
    }

    // Check for an interaction command with modifiers
    const interaction_bind_t bind = { mod, interaction };
    log::debug("Interaction: KeyPress ", bind.format());

    auto commandsIt = this->Bindings.find(bind);
    if (commandsIt == this->Bindings.end())
    {
      // Modifiers version not found, try ANY instead
      commandsIt = this->Bindings.find({ mod_t::ANY, interaction });
    }

    if (commandsIt != this->Bindings.end())
    {
      for (const std::string& command : commandsIt->second.CommandVector)
      {
        std::string commandWithArgs = command;
        if (!argsString.empty())
        {
          commandWithArgs.push_back(' ');
          commandWithArgs.append(argsString);
        };
        try
        {
          // XXX: Ignore the boolean return of triggerCommand,
          //  error is already logged by triggerCommand
          this->Interactor.triggerCommand(commandWithArgs);
        }
        catch (const f3d::interactor::command_runtime_exception& ex)
        {
          log::error(
            "Interaction: error running command: \"" + commandWithArgs + "\": " + ex.what());
        }
      }
    }

    // Always render after interaction
    this->Window.render();
  }

  //----------------------------------------------------------------------------
  void StartEventLoop(double deltaTime, std::function<void()> userCallBack)
  {
    // Trigger a render to ensure Window is ready to be configured
    this->Window.render();

    // Copy user callback
    this->EventLoopUserCallBack = std::move(userCallBack);

    // Configure UI delta time
    vtkRenderWindow* renWin = this->Window.GetRenderWindow();
    vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
    ren->SetUIDeltaTime(deltaTime);

    // Configure animation delta time
    this->AnimationManager->SetDeltaTime(deltaTime);

    // Create the timer
    this->EventLoopTimerId = this->VTKInteractor->CreateRepeatingTimer(deltaTime * 1000);

    // Create the callback and add an observer
    vtkNew<vtkCallbackCommand> timerCallBack;
    timerCallBack->SetCallback(
      [](vtkObject*, unsigned long, void* clientData, void*)
      {
        internals* that = static_cast<internals*>(clientData);
        that->EventLoop();
      });
    this->EventLoopObserverId =
      this->VTKInteractor->AddObserver(vtkCommand::TimerEvent, timerCallBack);
    timerCallBack->SetClientData(this);
  }

  //----------------------------------------------------------------------------
  void StopEventLoop()
  {
    this->VTKInteractor->RemoveObserver(this->EventLoopObserverId);
    this->VTKInteractor->DestroyTimer(this->EventLoopTimerId);
    this->EventLoopObserverId = -1;
    this->EventLoopTimerId = 0;
  }

  //----------------------------------------------------------------------------
  void EventLoop()
  {
    if (this->EventLoopUserCallBack)
    {
      this->EventLoopUserCallBack();
    }

    if (this->CommandBuffer.has_value())
    {
      this->Interactor.triggerCommand(this->CommandBuffer.value());
      this->CommandBuffer.reset();
    }

    this->AnimationManager->Tick();

    if (this->RenderRequested)
    {
      this->Window.render();
      this->RenderRequested = false;
    }
    else
    {
      this->Window.RenderUIOnly();
    }

    // Add camera orbit handling here till i find Application event loop
    if (this->Options.scene.camera.getOrbitTime() > 0)
    {
        double orbitTime = this->Options.scene.camera.getOrbitTime();
        double angle = 360.0 * (this->CurrentTime / orbitTime);
        // Calculate new camera position based on orbit
        double radius = 10.0; // Just Example Radius, can be adjusted later
        double x = radius * cos(vtkMath::RadiansFromDegrees(angle));
        double y = radius * sin(vtkMath::RadiansFromDegrees(angle));

        // Update camera position
        vtkCamera* camera =
            this->Window.GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
        camera->SetPosition(x, y, camera->GetPosition()[2]);
        camera->SetFocalPoint(0, 0, 0); // Example focal point, can be adjusted
    }
  }

  //----------------------------------------------------------------------------
  options& Options;
  window_impl& Window;
  scene_impl& Scene;
  interactor_impl& Interactor;
  animationManager* AnimationManager;

  vtkSmartPointer<vtkRenderWindowInteractor> VTKInteractor;
  vtkNew<vtkF3DInteractorStyle> Style;
  vtkSmartPointer<vtkF3DInteractorEventRecorder> Recorder;
  vtkNew<vtkF3DUIObserver> UIObserver;
  std::map<unsigned long, std::pair<int, std::function<void()>>> TimerCallBacks;

  std::map<std::string, std::function<void(const std::vector<std::string>&)>> Commands;
  std::optional<std::string> CommandBuffer;

  std::map<interaction_bind_t, BindingCommands> Bindings;
  std::multimap<std::string, interaction_bind_t> GroupedBinds;
  std::vector<std::string> OrderedBindGroups;

  vtkNew<vtkCellPicker> CellPicker;
  vtkNew<vtkPointPicker> PointPicker;

  int MiddleButtonDownPosition[2] = { 0, 0 };

  int DragDistanceTol = 3;      /* px */
  int TransitionDuration = 100; /* ms */

  std::function<void()> EventLoopUserCallBack = nullptr;
  unsigned long EventLoopTimerId = 0;
  int EventLoopObserverId = -1;
  std::atomic<bool> RenderRequested = false;
};

//----------------------------------------------------------------------------
interactor_impl::interactor_impl(options& options, window_impl& window, scene_impl& scene)
  : Internals(std::make_unique<interactor_impl::internals>(options, window, scene, *this))
{
  // scene need the interactor, scene will set the AnimationManager on the interactor
  this->Internals->Scene.SetInteractor(this);
  this->Internals->Window.SetInteractor(this);
  assert(this->Internals->AnimationManager);

  this->initCommands();
  this->initBindings();
}

//----------------------------------------------------------------------------
interactor_impl::~interactor_impl()
{
  vtkOutputWindow::GetInstance()->RemoveObservers(vtkF3DConsoleOutputWindow::TriggerEvent);
  vtkOutputWindow::GetInstance()->RemoveObservers(vtkF3DConsoleOutputWindow::ShowEvent);
  vtkOutputWindow::GetInstance()->RemoveObservers(vtkF3DConsoleOutputWindow::HideEvent);
}

//----------------------------------------------------------------------------
interactor& interactor_impl::initCommands()
{
  this->Internals->Commands.clear();

  const auto check_args =
    [&](const std::vector<std::string>& args, size_t expectedSize, std::string_view actionName)
  {
    if (args.size() != expectedSize)
    {
      throw interactor_impl::invalid_args_exception(std::string("Command: ") +
        std::string(actionName) + " is expecting " + std::to_string(expectedSize) + " arguments");
    }
  };

  // Add default callbacks
  this->addCommand("set",
    [&](const std::vector<std::string>& args)
    {
      check_args(args, 2, "set");
      this->Internals->Options.setAsString(args[0], args[1]);
    });

  this->addCommand("toggle",
    [&](const std::vector<std::string>& args)
    {
      check_args(args, 1, "toggle");
      this->Internals->Options.toggle(args[0]);
    });

  this->addCommand("reset",
    [&](const std::vector<std::string>& args)
    {
      check_args(args, 1, "reset");
      this->Internals->Options.reset(args[0]);
    });
  this->addCommand("clear",
    [&](const std::vector<std::string>& args)
    {
      check_args(args, 0, "clear");
#if F3D_MODULE_UI
      vtkF3DImguiConsole* console =
        vtkF3DImguiConsole::SafeDownCast(vtkOutputWindow::GetInstance());
      assert(console != nullptr);
      console->Clear();
#endif
