#include "interactor_impl.h"

#include "animationManager.h"
#include "log.h"
#include "scene_impl.h"
#include "utils.h"
#include "window_impl.h"

#include "vtkF3DConfigure.h"
#include "vtkF3DInteractorEventRecorder.h"
#include "vtkF3DInteractorStyle.h"
#include "vtkF3DRenderer.h"

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
#include <vector>

#include "camera.h"

namespace f3d::detail
{
class interactor_impl::internals
{
public:
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
  static void OnKeyPress(vtkObject*, unsigned long, void* clientData, void*)
  {
    internals* self = static_cast<internals*>(clientData);
    vtkRenderWindowInteractor* rwi = self->Style->GetInteractor();
    InteractionBind bind;
    bind.Interaction = rwi->GetKeySym();
    if (!bind.Interaction.empty())
    {
      // Make sure key symbols starts with an upper char (e.g. "space" -> "Space")
      bind.Interaction[0] = std::toupper(bind.Interaction[0]);
    }

    // Check for an interaction command with modifiers
    // XXX: Cannot use binary stuff here ?
    bind.Modifiers = ModifierKeys::NONE;
    const bool shift = rwi->GetShiftKey() == 1;
    const bool ctrl = rwi->GetControlKey() == 1;
    if (shift && ctrl)
    {
      bind.Modifiers = ModifierKeys::CTRL_SHIFT;
    }
    else if (ctrl == 1)
    {
      bind.Modifiers = ModifierKeys::CTRL;
    }
    else if (shift == 1)
    {
      bind.Modifiers = ModifierKeys::SHIFT;
    }
    // TODO: Add trace log for interactions
    auto commandsIt = self->InteractionCommands.find(bind);
    if (commandsIt == self->InteractionCommands.end())
    {
      // Modifiers version not found, try ANY instead
      bind.Modifiers = ModifierKeys::ANY;
      commandsIt = self->InteractionCommands.find(bind);
    }

    if (commandsIt != self->InteractionCommands.end())
    {
      for (const std::string& command : commandsIt->second)
      {
        if (!self->Interactor.triggerCommand(command))
        {
          log::error("Interaction: error running command:\"", command, "\", ignoring");
        }
      }
    }

    // Always render after interaction
    self->Window.render();
  }

  //----------------------------------------------------------------------------
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

    // No user defined behavior, load all files
    if (!filesVec.empty())
    {
      assert(self->AnimationManager);
      self->AnimationManager->StopAnimation();
      self->Scene.add(filesVec);
      self->Window.render();
    }
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

  //----------------------------------------------------------------------------
  std::function<bool(const std::vector<std::string>&)> DropFilesUserCallBack =
    [](const std::vector<std::string>&) { return false; };

  //----------------------------------------------------------------------------
  void StartInteractor()
  {
    this->VTKInteractor->Start();
  }

  //----------------------------------------------------------------------------
  void StopInteractor()
  {
    this->VTKInteractor->RemoveObservers(vtkCommand::TimerEvent);
    this->VTKInteractor->ExitCallback();
  }

  //----------------------------------------------------------------------------
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
  options& Options;
  window_impl& Window;
  scene_impl& Scene;
  interactor_impl& Interactor;
  animationManager* AnimationManager;

  vtkSmartPointer<vtkRenderWindowInteractor> VTKInteractor;
  vtkNew<vtkF3DInteractorStyle> Style;
  vtkSmartPointer<vtkF3DInteractorEventRecorder> Recorder;
  std::map<unsigned long, std::pair<int, std::function<void()>>> TimerCallBacks;

  std::map<std::string, std::function<bool(const std::vector<std::string>&)>> CommandCallbacks;

  struct InteractionBind
  {
    std::string Interaction;
    ModifierKeys Modifiers;

    bool operator<(const InteractionBind& bind) const
    {
      return this->Interaction < bind.Interaction ||
        (this->Interaction == bind.Interaction && this->Modifiers < bind.Modifiers);
    }
  };
  std::map<InteractionBind, std::vector<std::string>> InteractionCommands;

  vtkNew<vtkCellPicker> CellPicker;
  vtkNew<vtkPointPicker> PointPicker;

  int MiddleButtonDownPosition[2] = { 0, 0 };

  int DragDistanceTol = 3;      /* px */
  int TransitionDuration = 100; /* ms */
};

//----------------------------------------------------------------------------
interactor_impl::interactor_impl(options& options, window_impl& window, scene_impl& scene)
  : Internals(std::make_unique<interactor_impl::internals>(options, window, scene, *this))
{
  // scene need the interactor, scene will set the AnimationManager on the interactor
  this->Internals->Scene.SetInteractor(this);

  const auto check_args = [&](const std::vector<std::string>& args, size_t expectedSize,
                            std::string_view actionName) -> bool
  {
    if (args.size() != expectedSize)
    {
      log::error("Command: ", actionName, " is expecting ", std::to_string(expectedSize),
        " arguments, ignoring");
      return false;
    }
    return true;
  };

  // Add default callbacks
  this->addCommandCallback("set",
    [&](const std::vector<std::string>& args) -> bool
    {
      if (!check_args(args, 2, "set"))
      {
        return false;
      }
      this->Internals->Options.setAsString(args[0], args[1]);
      return true;
    });

  this->addCommandCallback("toggle",
    [&](const std::vector<std::string>& args) -> bool
    {
      if (!check_args(args, 1, "toggle"))
      {
        return false;
      }
      this->Internals->Options.toggle(args[0]);
      return true;
    });

  this->addCommandCallback("reset",
    [&](const std::vector<std::string>& args) -> bool
    {
      if (!check_args(args, 1, "reset"))
      {
        return false;
      }
      this->Internals->Options.reset(args[0]);
      return true;
    });
  this->addCommandCallback("print",
    [&](const std::vector<std::string>& args) -> bool
    {
      if (!check_args(args, 1, "print"))
      {
        return false;
      }
      log::info(this->Internals->Options.getAsString(args[0]));
      return true;
    });

  this->addCommandCallback("cycle_animation",
    [&](const std::vector<std::string>&) -> bool
    {
      vtkRenderWindow* renWin = this->Internals->Window.GetRenderWindow();
      vtkF3DRenderer* ren =
        vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
      this->Internals->AnimationManager->CycleAnimation();
      this->Internals->Options.scene.animation.index =
        this->Internals->AnimationManager->GetAnimationIndex();
      ren->SetAnimationnameInfo(this->Internals->AnimationManager->GetAnimationName());
      return true;
    });

  this->addCommandCallback("cycle_coloring",
    [&](const std::vector<std::string>& args) -> bool
    {
      if (!check_args(args, 1, "cycle_coloring"))
      {
        return false;
      }

      std::string_view type = args[0];
      vtkRenderWindow* renWin = this->Internals->Window.GetRenderWindow();
      vtkF3DRenderer* ren =
        vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
      if (type == "field")
      {
        ren->CycleFieldForColoring();
      }
      else if (type == "array")
      {
        ren->CycleArrayForColoring();
      }
      else if (type == "component")
      {
        ren->CycleComponentForColoring();
      }
      else
      {
        log::error("Command: cycle_coloring arg:\"", type, "\" is not recognized, ignoring");
        return false;
      }
      this->Internals->SynchronizeScivisOptions(this->Internals->Options, ren);
      this->Internals->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
      return true;
    });

  this->addCommandCallback("roll_camera",
    [&](const std::vector<std::string>& args) -> bool
    {
      if (!check_args(args, 1, "roll_camera"))
      {
        return false;
      }
      this->Internals->Window.getCamera().roll(options::parse<int>(args[0]));
      return true;
    });

  this->addCommandCallback("increase_light_intensity",
    [&](const std::vector<std::string>&) -> bool
    {
      this->Internals->IncreaseLightIntensity(false);
      return true;
    });

  this->addCommandCallback("decrease_light_intensity",
    [&](const std::vector<std::string>&) -> bool
    {
      this->Internals->IncreaseLightIntensity(true);
      return true;
    });

  this->addCommandCallback("print_scene_info",
    [&](const std::vector<std::string>&) -> bool
    {
      this->Internals->Window.PrintColoringDescription(log::VerboseLevel::INFO);
      this->Internals->Window.PrintSceneDescription(log::VerboseLevel::INFO);
      return true;
    });

  this->addCommandCallback("set_camera",
    [&](const std::vector<std::string>& args) -> bool
    {
      if (!check_args(args, 1, "cycle_coloring"))
      {
        return false;
      }

      std::string_view type = args[0];
      if (type == "front")
      {
        this->Internals->SetViewOrbit(internals::ViewType::VT_FRONT, this->Internals.get());
      }
      else if (type == "top")
      {
        this->Internals->SetViewOrbit(internals::ViewType::VT_TOP, this->Internals.get());
      }
      else if (type == "right")
      {
        this->Internals->SetViewOrbit(internals::ViewType::VT_RIGHT, this->Internals.get());
      }
      else if (type == "isometric")
      {
        this->Internals->SetViewOrbit(internals::ViewType::VT_ISOMETRIC, this->Internals.get());
      }
      else
      {
        log::error("Command: set_camera arg:\"", type, "\" is not recognized, ignoring");
        return false;
      }
      return true;
    });

  this->addCommandCallback("toggle_volume_rendering",
    [&](const std::vector<std::string>&) -> bool
    {
      this->Internals->Options.model.volume.enable = !this->Internals->Options.model.volume.enable;
      this->Internals->Window.render();
      this->Internals->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
      return true;
    });

  this->addCommandCallback("toggle_fps",
    [&](const std::vector<std::string>&) -> bool
    {
      this->Internals->Options.ui.fps = !this->Internals->Options.ui.fps;
      this->Internals->Window.render();
      return true;
    });

  this->addCommandCallback("stop_interactor",
    [&](const std::vector<std::string>&) -> bool
    {
      this->Internals->StopInteractor();
      return true;
    });

  this->addCommandCallback("reset_camera",
    [&](const std::vector<std::string>&) -> bool
    {
      this->Internals->Window.getCamera().resetToDefault();
      return true;
    });

  this->addCommandCallback("toggle_animation",
    [&](const std::vector<std::string>&) -> bool
    {
      this->Internals->AnimationManager->ToggleAnimation();
      return true;
    });

  // Available standard keys: None
  this->addInteractionCommands("W", ModifierKeys::NONE, { "cycle_animation" });
  this->addInteractionCommands("C", ModifierKeys::NONE, { "cycle_coloring field" });
  this->addInteractionCommands("S", ModifierKeys::NONE, { "cycle_coloring array" });
  this->addInteractionCommands("Y", ModifierKeys::NONE, { "cycle_coloring component" });
  this->addInteractionCommands("B", ModifierKeys::NONE, { "toggle ui.scalar_bar" });
  this->addInteractionCommands(
    "P", ModifierKeys::NONE, { "toggle render.effect.translucency_support" });
  this->addInteractionCommands(
    "Q", ModifierKeys::NONE, { "toggle render.effect.ambient_occlusion" });
  this->addInteractionCommands("A", ModifierKeys::NONE, { "toggle render.effect.anti_aliasing" });
  this->addInteractionCommands("T", ModifierKeys::NONE, { "toggle render.effect.tone_mapping" });
  this->addInteractionCommands("E", ModifierKeys::NONE, { "toggle render.show_edges" });
  this->addInteractionCommands("X", ModifierKeys::NONE, { "toggle interactor.axis" });
  this->addInteractionCommands("G", ModifierKeys::NONE, { "toggle render.grid.enable" });
  this->addInteractionCommands("N", ModifierKeys::NONE, { "toggle ui.filename" });
  this->addInteractionCommands("M", ModifierKeys::NONE, { "toggle ui.metadata" });
  this->addInteractionCommands("Z", ModifierKeys::NONE, { "toggle_fps" });
  this->addInteractionCommands("R", ModifierKeys::NONE, { "toggle render.raytracing.enable" });
  this->addInteractionCommands("D", ModifierKeys::NONE, { "toggle render.raytracing.denoise" });
  this->addInteractionCommands("V", ModifierKeys::NONE, { "toggle_volume_rendering" });
  this->addInteractionCommands("I", ModifierKeys::NONE, { "toggle model.volume.inverse" });
  this->addInteractionCommands("O", ModifierKeys::NONE, { "toggle model.point_sprites.enable" });
  this->addInteractionCommands("U", ModifierKeys::NONE, { "toggle render.background.blur" });
  this->addInteractionCommands("K", ModifierKeys::NONE, { "toggle interactor.trackball" });
  this->addInteractionCommands("F", ModifierKeys::NONE, { "toggle render.hdri.ambient" });
  this->addInteractionCommands("J", ModifierKeys::NONE, { "toggle render.background.skybox" });
  this->addInteractionCommands("L", ModifierKeys::NONE, { "increase_light_intensity" });
  this->addInteractionCommands("L", ModifierKeys::SHIFT, { "decrease_light_intensity" });
  this->addInteractionCommands("H", ModifierKeys::NONE, { "toggle ui.cheatsheet" });
  this->addInteractionCommands("Question", ModifierKeys::ANY, { "print_scene_info" });
  this->addInteractionCommands("1", ModifierKeys::ANY, { "set_camera front" });
  this->addInteractionCommands("3", ModifierKeys::ANY, { "set_camera right" });
  this->addInteractionCommands("4", ModifierKeys::ANY, { "roll_camera -90" });
  this->addInteractionCommands("5", ModifierKeys::ANY, { "toggle scene.camera.orthographic" });
  this->addInteractionCommands("6", ModifierKeys::ANY, { "roll_camera 90" });
  this->addInteractionCommands("7", ModifierKeys::ANY, { "set_camera top" });
  this->addInteractionCommands("9", ModifierKeys::ANY, { "set_camera isometric" });
  this->addInteractionCommands(F3D_EXIT_HOTKEY_SYM, ModifierKeys::NONE, { "stop_interactor" });
  this->addInteractionCommands("Return", ModifierKeys::NONE, { "reset_camera" });
  this->addInteractionCommands("Space", ModifierKeys::NONE, { "toggle_animation" });
}

//----------------------------------------------------------------------------
interactor_impl::~interactor_impl() = default;

//----------------------------------------------------------------------------
interactor& interactor_impl::setDropFilesCallBack(
  std::function<bool(std::vector<std::string>)> callBack)
{
  this->Internals->DropFilesUserCallBack = callBack;
  return *this;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::addCommandCallback(
  std::string action, std::function<bool(const std::vector<std::string>&)> callback)
{
  this->Internals->CommandCallbacks[std::move(action)] = callback;
  return *this;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::removeCommandCallback(const std::string& action)
{
  this->Internals->CommandCallbacks.erase(action);
  return *this;
}

//----------------------------------------------------------------------------
bool interactor_impl::triggerCommand(std::string_view command)
{
  // TODO: add trace log for command
  std::vector<std::string> tokens;
  try
  {
    tokens = utils::tokenize(command);
  }
  catch (const utils::tokenize_exception&)
  {
    log::error("Command: unable to tokenize command:\"", command, "\", ignoring");
    return false;
  }

  const std::string& action = tokens[0];
  try
  {
    // Find the right command to call
    auto callbackIt = this->Internals->CommandCallbacks.find(action);
    if (callbackIt != this->Internals->CommandCallbacks.end())
    {
      return callbackIt->second({ tokens.begin() + 1, tokens.end() });
    }
    else
    {
      log::error("Command: \"", action, "\" is not recognized, ignoring");
      return false;
    }
  }
  catch (const f3d::options::incompatible_exception&)
  {
    log::error("Command: provided args in command: \"", command,
      "\" are not compatible with action:\"", action, "\", ignoring");
  }
  catch (const f3d::options::inexistent_exception&)
  {
    log::error("Command: provided args in command: \"", command,
      "\" point to an inexistent option, ignoring");
  }
  catch (const f3d::options::no_value_exception&)
  {
    log::error("Command: provided args in command: \"", command,
      "\" point to an option without a value, ignoring");
  }
  catch (const f3d::options::parsing_exception&)
  {
    log::error("Command: provided args in command: \"", command,
      "\" cannot be parsed into an option, ignoring");
  }
  return false;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::addInteractionCommands(
  std::string interaction, ModifierKeys modifiers, const std::vector<std::string>& commands)
{
  this->Internals->InteractionCommands[{ std::move(interaction), std::move(modifiers) }] = commands;
  return *this;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::addInteractionCommand(
  std::string interaction, ModifierKeys modifiers, const std::string& command)
{
  this->Internals->InteractionCommands[{ std::move(interaction), std::move(modifiers) }] = {
    command
  };
  return *this;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::removeInteractionCommands(
  std::string interaction, ModifierKeys modifiers)
{
  this->Internals->InteractionCommands.erase({ std::move(interaction), std::move(modifiers) });
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
