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
#include <regex>
#include <vector>

#include "camera.h"

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
  options& Options;
  window_impl& Window;
  scene_impl& Scene;
  interactor_impl& Interactor;
  animationManager* AnimationManager;

  vtkSmartPointer<vtkRenderWindowInteractor> VTKInteractor;
  vtkNew<vtkF3DInteractorStyle> Style;
  vtkSmartPointer<vtkF3DInteractorEventRecorder> Recorder;
  std::map<unsigned long, std::pair<int, std::function<void()>>> TimerCallBacks;

  std::map<std::string, std::function<void(const std::vector<std::string>&)>> Commands;

  std::map<interaction_bind_t, BindingCommands> Bindings;
  std::multimap<std::string, interaction_bind_t> GroupedBinds;
  std::vector<std::string> OrderedBindGroups;

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
  this->Internals->Window.SetInteractor(this);
  assert(this->Internals->AnimationManager);

  this->initCommands();
  this->initBindings();
}

//----------------------------------------------------------------------------
interactor_impl::~interactor_impl() = default;

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
  this->addCommand("print",
    [&](const std::vector<std::string>& args)
    {
      check_args(args, 1, "print");
      log::info(this->Internals->Options.getAsString(args[0]));
    });

  this->addCommand("cycle_animation",
    [&](const std::vector<std::string>&)
    {
      this->Internals->AnimationManager->CycleAnimation();
      this->Internals->Options.scene.animation.index =
        this->Internals->AnimationManager->GetAnimationIndex();
    });

  this->addCommand("cycle_coloring",
    [&](const std::vector<std::string>& args)
    {
      check_args(args, 1, "cycle_coloring");
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
        throw interactor_impl::invalid_args_exception(
          std::string("Command: cycle_coloring arg:\"") + std::string(type) +
          "\" is not recognized.");
      }
      this->Internals->SynchronizeScivisOptions(this->Internals->Options, ren);
      this->Internals->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
    });

  this->addCommand("roll_camera",
    [&](const std::vector<std::string>& args)
    {
      check_args(args, 1, "roll_camera");
      this->Internals->Window.getCamera().roll(options::parse<int>(args[0]));
    });

  this->addCommand("increase_light_intensity",
    [&](const std::vector<std::string>&) { this->Internals->IncreaseLightIntensity(false); });

  this->addCommand("decrease_light_intensity",
    [&](const std::vector<std::string>&) { this->Internals->IncreaseLightIntensity(true); });

  this->addCommand("print_scene_info",
    [&](const std::vector<std::string>&)
    {
      this->Internals->Window.PrintColoringDescription(log::VerboseLevel::INFO);
      this->Internals->Window.PrintSceneDescription(log::VerboseLevel::INFO);
    });

  this->addCommand("set_camera",
    [&](const std::vector<std::string>& args)
    {
      check_args(args, 1, "cycle_coloring");
      std::string_view type = args[0];
      if (type == "front")
      {
        this->Internals->SetViewOrbit(internals::ViewType::VT_FRONT);
      }
      else if (type == "top")
      {
        this->Internals->SetViewOrbit(internals::ViewType::VT_TOP);
      }
      else if (type == "right")
      {
        this->Internals->SetViewOrbit(internals::ViewType::VT_RIGHT);
      }
      else if (type == "isometric")
      {
        this->Internals->SetViewOrbit(internals::ViewType::VT_ISOMETRIC);
      }
      else
      {
        throw interactor_impl::invalid_args_exception(
          std::string("Command: set_camera arg:\"") + std::string(type) + "\" is not recognized.");
      }
    });

  this->addCommand("toggle_volume_rendering",
    [&](const std::vector<std::string>&)
    {
      this->Internals->Options.model.volume.enable = !this->Internals->Options.model.volume.enable;
      this->Internals->Window.render();
      this->Internals->Window.PrintColoringDescription(log::VerboseLevel::DEBUG);
    });

  this->addCommand("toggle_fps",
    [&](const std::vector<std::string>&)
    {
      this->Internals->Options.ui.fps = !this->Internals->Options.ui.fps;
      this->Internals->Window.render();
    });

  this->addCommand(
    "stop_interactor", [&](const std::vector<std::string>&) { this->Internals->StopInteractor(); });

  this->addCommand("reset_camera",
    [&](const std::vector<std::string>&) { this->Internals->Window.getCamera().resetToDefault(); });

  this->addCommand("toggle_animation",
    [&](const std::vector<std::string>&) { this->Internals->AnimationManager->ToggleAnimation(); });

  this->addCommand("add_files",
    [&](const std::vector<std::string>& files)
    {
      this->Internals->AnimationManager->StopAnimation();
      this->Internals->Scene.add(files);
    });
  return *this;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::addCommand(
  const std::string& action, std::function<void(const std::vector<std::string>&)> callback)
{
  const auto [it, success] = this->Internals->Commands.insert({ action, callback });
  if (!success)
  {
    throw interactor::already_exists_exception(
      "Could not add a command callback for action: " + action + " as it already exists.");
  }
  return *this;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::removeCommand(const std::string& action)
{
  this->Internals->Commands.erase(action);
  return *this;
}

//----------------------------------------------------------------------------
std::vector<std::string> interactor_impl::getCommandActions() const
{
  std::vector<std::string> actions;
  for (auto const& [action, callback] : this->Internals->Commands)
  {
    actions.emplace_back(action);
  }
  return actions;
}

//----------------------------------------------------------------------------
bool interactor_impl::triggerCommand(std::string_view command)
{
  log::debug("Command: ", command);
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
    auto callbackIt = this->Internals->Commands.find(action);
    if (callbackIt != this->Internals->Commands.end())
    {
      callbackIt->second({ tokens.begin() + 1, tokens.end() });
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
  catch (const invalid_args_exception& ex)
  {
    log::error(ex.what(), " Ignoring.");
  }
  catch (const std::exception& ex)
  {
    throw interactor::command_runtime_exception(ex.what());
  }
  return false;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::initBindings()
{
  this->Internals->Bindings.clear();
  this->Internals->GroupedBinds.clear();
  this->Internals->OrderedBindGroups.clear();
  f3d::options& opts = this->Internals->Options;

  // clang-format off
  // Define lambdas used for documentation

  // Shorten a long name
  auto shortName = [](const std::string& name, int maxChar)
  {
    if (name.size() <= static_cast<size_t>(maxChar) || maxChar <= 3)
    {
      return name;
    }
    else
    {
      return name.substr(0, maxChar - 3) + "...";
    }
  };

  // "doc", ""
  auto docStr = [](const std::string& doc)
  { return std::pair(doc, ""); };

  // "doc", "valueString()"
  auto docStrVal = [](const std::string& doc, const std::function<std::string()>& valueString)
  { return std::pair(doc, valueString());  };

  // "value"
  auto docDbl = [](double val, int precision)
  {
    std::stringstream valStream;
    valStream.precision(precision);
    valStream << std::fixed;
    valStream << val;
    return valStream.str();
  };

  // "ON/OFF"
  auto docTgl = [](bool val)
  { return val ? "ON" : "OFF"; };

  // "ON/OFF/NO SET"
  auto docTglOpt = [](std::optional<bool> val)
  { return val.has_value() ? (val.value() ? "ON" : "OFF") : "N/A"; };

  // "animationName"
  auto animStr = [&]()
  { return this->Internals->AnimationManager->GetAnimationName(); };

  // "POINT/CELL"
  auto pointCellStr = [&]()
  { return opts.model.scivis.cells ? "CELL" : "POINT"; };

  // "arrayName"
  auto arrayStr = [&]()
  {
    return opts.model.scivis.array_name.has_value()
      ? shortName(opts.model.scivis.array_name.value(), 15) + (opts.model.scivis.enable ? "" : " (forced)")
      : "OFF";
  };

  // "component"
  auto compStr = [&]()
  {
    vtkRenderWindow* renWin = this->Internals->Window.GetRenderWindow();
    vtkF3DRenderer* ren =
      vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
    return ren->ComponentToString(opts.model.scivis.component);
  };

  // Available standard keys: None
  this->addBinding({mod_t::NONE, "W"}, "cycle_animation", "Scene", std::bind(docStrVal, "Cycle animation", animStr));
  this->addBinding({mod_t::NONE, "C"}, "cycle_coloring field", "Scene", std::bind(docStrVal, "Cycle point/cell data coloring", pointCellStr));
  this->addBinding({mod_t::NONE, "S"}, "cycle_coloring array", "Scene", std::bind(docStrVal, "Cycle array to color with", arrayStr));
  this->addBinding({mod_t::NONE, "Y"}, "cycle_coloring component", "Scene", std::bind(docStrVal, "Cycle component to color with", compStr));
  this->addBinding({mod_t::NONE, "B"}, "toggle ui.scalar_bar", "Scene", std::bind(docStrVal, "Toggle the scalar bar display",
    [&]() { return docTgl(opts.ui.scalar_bar);}));
  this->addBinding({mod_t::NONE, "P"}, "toggle render.effect.translucency_support", "Scene", std::bind(docStrVal, "Toggle scalar bar display",
    [&]() { return docTgl(opts.render.effect.translucency_support);}));
  this->addBinding({mod_t::NONE, "Q"}, "toggle render.effect.ambient_occlusion","Scene", std::bind(docStrVal, "Toggle ambient occlusion",
    [&]() { return docTgl(opts.render.effect.ambient_occlusion);}));
  this->addBinding({mod_t::NONE, "A"}, "toggle render.effect.anti_aliasing","Scene", std::bind(docStrVal, "Toggle anti-aliasing",
    [&]() { return docTgl(opts.render.effect.anti_aliasing);}));
  this->addBinding({mod_t::NONE, "T"}, "toggle render.effect.tone_mapping","Scene", std::bind(docStrVal, "Toggle tone mapping",
    [&]() { return docTgl(opts.render.effect.tone_mapping);}));
  this->addBinding({mod_t::NONE, "E"}, "toggle render.show_edges","Scene", std::bind(docStrVal, "Toggle edges display",
    [&]() { return docTglOpt(opts.render.show_edges);}));
  this->addBinding({mod_t::NONE, "X"}, "toggle interactor.axis","Scene", std::bind(docStrVal, "Toggle axes display",
    [&]() { return docTgl(opts.interactor.axis);}));
  this->addBinding({mod_t::NONE, "G"}, "toggle render.grid.enable","Scene", std::bind(docStrVal, "Toggle grid display",
    [&]() { return docTgl(opts.render.grid.enable);}));
  this->addBinding({mod_t::NONE, "N"}, "toggle ui.filename","Scene", std::bind(docStrVal, "Toggle filename display",
    [&]() { return docTgl(opts.ui.filename);}));
  this->addBinding({mod_t::NONE, "M"}, "toggle ui.metadata","Scene", std::bind(docStrVal, "Toggle metadata display",
    [&]() { return docTgl(opts.ui.metadata);}));
  this->addBinding({mod_t::NONE, "Z"}, "toggle_fps","Scene", std::bind(docStrVal, "Toggle FPS counter display",
    [&]() { return docTgl(opts.ui.fps);}));
#if F3D_MODULE_RAYTRACING
  this->addBinding({mod_t::NONE, "R"}, "toggle render.raytracing.enable","Scene", std::bind(docStrVal, "Toggle raytracing rendering",
    [&]() { return docTgl(opts.render.raytracing.enable);}));
  this->addBinding({mod_t::NONE, "D"}, "toggle render.raytracing.denoise","Scene", std::bind(docStrVal, "Toggle denoising when raytracing",
    [&]() { return docTgl(opts.render.raytracing.denoise);}));
#endif
  this->addBinding({mod_t::NONE, "V"}, "toggle_volume_rendering","Scene", std::bind(docStrVal, "Toggle volume rendering",
    [&]() { return docTgl(opts.model.volume.enable);}));
  this->addBinding({mod_t::NONE, "I"}, "toggle model.volume.inverse","Scene", std::bind(docStrVal, "Toggle inverse volume opacity",
    [&]() { return docTgl(opts.model.volume.inverse);}));
  this->addBinding({mod_t::NONE, "O"}, "toggle model.point_sprites.enable","Scene", std::bind(docStrVal, "Toggle point sprites rendering",
    [&]() { return docTgl(opts.model.point_sprites.enable);}));
  this->addBinding({mod_t::NONE, "U"}, "toggle render.background.blur","Scene", std::bind(docStrVal, "Toggle blur background",
    [&]() { return docTgl(opts.render.background.blur);}));
  this->addBinding({mod_t::NONE, "K"}, "toggle interactor.trackball","Scene", std::bind(docStrVal, "Toggle trackball interaction",
    [&]() { return docTgl(opts.interactor.trackball);}));
  this->addBinding({mod_t::NONE, "F"}, "toggle render.hdri.ambient","Scene", std::bind(docStrVal, "Toggle HDRI ambient lighting",
    [&]() { return docTgl(opts.render.hdri.ambient);}));
  this->addBinding({mod_t::NONE, "J"}, "toggle render.background.skybox","Scene", std::bind(docStrVal, "Toggle HDRI skybox",
    [&]() { return docTgl(opts.render.background.skybox);}));
  this->addBinding({mod_t::NONE, "L"}, "increase_light_intensity", "Scene", std::bind(docStrVal, "Increase lights intensity",
    [&]() { return docDbl(opts.render.light.intensity, 2);}));
  this->addBinding({mod_t::SHIFT, "L"}, "decrease_light_intensity", "Scene", std::bind(docStrVal, "Decrease lights intensity",
    [&]() { return docDbl(opts.render.light.intensity, 2);}));
  this->addBinding({mod_t::ANY, "1"}, "set_camera front", "Camera", std::bind(docStr, "Front View camera"));
  this->addBinding({mod_t::ANY, "3"}, "set_camera right", "Camera", std::bind(docStr, "Right View camera"));
  this->addBinding({mod_t::ANY, "4"}, "roll_camera -90", "Camera", std::bind(docStr, "Rotate camera right"));
  this->addBinding({mod_t::ANY, "5"}, "toggle scene.camera.orthographic", "Camera", std::bind(docStrVal, "Toggle Orthographic Projection",
    [&]() { return docTglOpt(opts.scene.camera.orthographic);}));
  this->addBinding({mod_t::ANY, "6"}, "roll_camera 90", "Camera", std::bind(docStr, "Rotate camera left"));
  this->addBinding({mod_t::ANY, "7"}, "set_camera top", "Camera", std::bind(docStr, "Top View camera"));
  this->addBinding({mod_t::ANY, "9"}, "set_camera isometric", "Camera", std::bind(docStr, "Isometric View camera"));
  this->addBinding({mod_t::NONE, "H"}, "toggle ui.cheatsheet", "Others", std::bind(docStr, "Toggle cheatsheet display"));
  this->addBinding({mod_t::ANY, "Question"}, "print_scene_info", "Others", std::bind(docStr, "Print scene descr to terminal"));
  this->addBinding({mod_t::NONE, F3D_EXIT_HOTKEY_SYM}, "stop_interactor", "Others", std::bind(docStr, "Quit"));
  this->addBinding({mod_t::NONE, "Return"}, "reset_camera", "Others", std::bind(docStr, "Reset camera to initial parameters"));
  this->addBinding({mod_t::NONE, "Space"}, "toggle_animation", "Others", std::bind(docStr, "Play/Pause animation if any"));
  this->addBinding({mod_t::NONE, "Drop"}, "add_files", "Others", std::bind(docStr, "Add files to the scene"));
  // clang-format on

  return *this;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::addBinding(const interaction_bind_t& bind,
  std::vector<std::string> commands, std::string group,
  documentation_callback_t documentationCallback)
{
  const auto [it, success] = this->Internals->Bindings.insert(
    { bind, { std::move(commands), std::move(documentationCallback) } });
  if (!success)
  {
    throw interactor::already_exists_exception(
      "Could not add interaction commands for interaction: " + bind.format() +
      " as it already exists.");
  }
  else
  {
    // Add the bind to the group
    auto groupIt = this->Internals->GroupedBinds.emplace(std::move(group), bind);
    if (this->Internals->GroupedBinds.count(groupIt->first) == 1)
    {
      // Add the group in order if first addition
      this->Internals->OrderedBindGroups.emplace_back(groupIt->first);
    }
  }
  return *this;
}

//----------------------------------------------------------------------------
interactor& interactor_impl::addBinding(const interaction_bind_t& bind, std::string command,
  std::string group, documentation_callback_t documentationCallback)
{
  return this->addBinding(bind, std::vector<std::string>{ std::move(command) }, std::move(group),
    std::move(documentationCallback));
}

//----------------------------------------------------------------------------
interactor& interactor_impl::removeBinding(const interaction_bind_t& bind)
{
  this->Internals->Bindings.erase(bind);

  // Look for the group of the removed bind
  std::string group;
  for (auto it = this->Internals->GroupedBinds.begin(); it != this->Internals->GroupedBinds.end();
       it++)
  {
    if (it->second == bind)
    {
      // Binds are unique
      // Erase the bind entry in the group
      group = it->first;
      this->Internals->GroupedBinds.erase(it);
      if (this->Internals->GroupedBinds.count(group) == 0)
      {
        // If it was the last one, remove it from the ordered group
        // We know the group is present and unique in the vector, so only erase once
        auto vecIt = std::find(this->Internals->OrderedBindGroups.begin(),
          this->Internals->OrderedBindGroups.end(), group);
        assert(vecIt != this->Internals->OrderedBindGroups.end());
        this->Internals->OrderedBindGroups.erase(vecIt);
      }
      break;
    }
  }
  return *this;
}

//----------------------------------------------------------------------------
std::vector<std::string> interactor_impl::getBindGroups() const
{
  return this->Internals->OrderedBindGroups;
}

//----------------------------------------------------------------------------
std::vector<interaction_bind_t> interactor_impl::getBindsForGroup(std::string group) const
{
  std::vector<interaction_bind_t> output;
  for (auto [it, rangeEnd] = this->Internals->GroupedBinds.equal_range(group); it != rangeEnd; ++it)
  {
    output.emplace_back(it->second);
  }
  return output;
}

//----------------------------------------------------------------------------
std::pair<std::string, std::string> interactor_impl::getBindingDocumentation(
  const interaction_bind_t& bind) const
{
  std::vector<std::tuple<std::string, std::string, std::string>> doc;
  auto docFunc = this->Internals->Bindings[bind].DocumentationCallback;
  return docFunc ? docFunc() : std::make_pair(std::string(), std::string());
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

//----------------------------------------------------------------------------
interactor_impl::invalid_args_exception::invalid_args_exception(const std::string& what)
  : exception(what)
{
}
}
