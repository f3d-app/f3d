#ifndef f3d_commands_doc_h
#define f3d_commands_doc_h

#include <array>
#include <cstddef>
#include <functional>
#include <string_view>

struct InteractiveCommand
{
  using ArgDoc = std::pair<std::string, std::string>;
  std::string Name;
  std::string Documentation;
  std::vector<ArgDoc> ArgsDocumentation;
  std::function<void(const std::vector<std::string>&)> Handler;
};

template<std::size_t N>
struct InteractiveCommandDoc
{
  using StringViewPair = std::pair<std::string_view, std::string_view>;
  std::string_view Name;
  std::string_view Documentation;
  StringViewPair Arguments[N];
};

constexpr InteractiveCommandDoc<2> F3D_CMD_SET = { "set", "Set libf3d option",
  { { "option.name", "Name of option to set" }, { "value", "New value for option" } } };

constexpr InteractiveCommandDoc<1> F3D_CMD_TOGGLE = { "toggle", "Toggle a boolean libf3d option",
  { { "option.name", "Name of a boolean option to toggle" } } };

constexpr InteractiveCommandDoc<1> F3D_CMD_RESET = { "reset",
  "Reset a libf3d option to its default value",
  { { "option.name", "Name of option to reset to default value" } } };

constexpr InteractiveCommandDoc<0> F3D_CMD_CLEAR = { "clear",
  "A command to clear console. No argument.", {} };

constexpr InteractiveCommandDoc<1> F3D_CMD_PRINT = { "print", "Print the value of an libf3d option",
  { { "option.name", "Name of option to be printed" } } };

constexpr InteractiveCommandDoc<2> F3D_CMD_SET_READER_OPTION = { "set_reader_option",
  "Set reader option",
  { { "option.name", "Name of option to set" }, { "value", "New value for option" } } };

constexpr InteractiveCommandDoc<0> F3D_CMD_CYCLE_ANTI_ALIASING = { "cycle_anti_aliasing",
  "A command to cycle between the anti-aliasing method (none,fxaa,ssaa).", {} };

constexpr InteractiveCommandDoc<0> F3D_CMD_CYCLE_ANIMATION = { "cycle_animation",
  "A specific command to cycle scene.animation.index option using model information, No argument.",
  {} };

constexpr InteractiveCommandDoc<1> F3D_CMD_CYCLE_COLORING = { "cycle_coloring",
  "A specific command to manipulate scivis options using model information. Supports field, array "
  "or component arguments, see documentation.",
  { { "field/array/component", "Specific field, array, or component argument" } } };

constexpr InteractiveCommandDoc<1> F3D_CMD_ROLL_CAMERA = { "roll_camera",
  "A specific command to roll the camera on its side, takes an angle in degrees as an argument.",
  { { "value", "Angle in degrees to roll the camera" } } };

constexpr InteractiveCommandDoc<0> F3D_CMD_INCREASE_LIGHT_INTENSITY = { "increase_light_intensity",
  "A specific command to increase light intensity." };
constexpr InteractiveCommandDoc<0> F3D_CMD_DECREASE_LIGHT_INTENSITY = { "decrease_light_intensity",
  "A specific command to decrease light intensity." };
constexpr InteractiveCommandDoc<0> F3D_CMD_INCREASE_OPACITY = {
  "increase_opacity",
  "A specific command to increase opacity. Unset opacity will be treated as if it has a value of 1."
};
constexpr InteractiveCommandDoc<0> F3D_CMD_DECREASE_OPACITY = {
  "decrease_opacity",
  "A specific command to decrease opacity. Unset opacity will be treated as if it has a value of 1."
};
constexpr InteractiveCommandDoc<0> F3D_CMD_PRINT_SCENE_INFO = { "print_scene_info",
  "A specific command to print information about the scene." };
constexpr InteractiveCommandDoc<0> F3D_CMD_PRINT_COLORING_INFO = { "print_coloring_info",
  "A specific command to print information about coloring settings." };
constexpr InteractiveCommandDoc<0> F3D_CMD_PRINT_MESH_INFO = { "print_mesh_info",
  "A specific command to print information from the mesh importer." };
constexpr InteractiveCommandDoc<0> F3D_CMD_PRINT_OPTIONS_INFO = { "print_options_info",
  "A specific command to print configuration options that have a value." };

constexpr InteractiveCommandDoc<1> F3D_CMD_SET_CAMERA = { "set_camera",
  "A specific command to position the camera in the specified location relative to the model. "
  "Supports front, top, right, isometric arguments.",
  { { "front/top/right/isometric", "Camera position relative to the model" } } };

constexpr InteractiveCommandDoc<0> F3D_CMD_TOGGLE_VOLUME_RENDERING = { "toggle_volume_rendering",
  "A specific command to toggle model.volume.enable and print coloring information." };
constexpr InteractiveCommandDoc<0> F3D_CMD_STOP_INTERACTOR = { "stop_interactor",
  "A specific command to stop the interactor hence quitting the application." };
constexpr InteractiveCommandDoc<0> F3D_CMD_RESET_CAMERA = { "reset_camera",
  "A specific command to reset the camera to its original location." };
constexpr InteractiveCommandDoc<0> F3D_CMD_TOGGLE_ANIMATION = { "toggle_animation",
  "A specific command to start/stop the animation." };

constexpr InteractiveCommandDoc<1> F3D_CMD_ADD_FILES = { "add_files",
  "A specific command to add files to the scene (overridden by a F3D specific command, see below). "
  "Take one or more files as arguments.",
  { { "[path/to/file1] [path/to/file2]", "One or more files to add to the scene" } } };

constexpr InteractiveCommandDoc<0> F3D_CMD_EXIT = { "exit",
  "A specific command to quit the application." };

constexpr InteractiveCommandDoc<1> F3D_CMD_LOAD_PREVIOUS_FILE_GROUP = { "load_previous_file_group",
  "A specific command to load the previous file or file group. Camera state is kept if keep_camera "
  "is true (default: false).",
  { { "[keep_camera]", "Optional flag to keep camera state" } } };

constexpr InteractiveCommandDoc<1> F3D_CMD_LOAD_NEXT_FILE_GROUP = { "load_next_file_group",
  "A specific command to load the next file or file group. Camera state is kept if keep_camera is "
  "true (default: false).",
  { { "[keep_camera]", "Optional flag to keep camera state" } } };

constexpr InteractiveCommandDoc<0> F3D_CMD_RELOAD_CURRENT_FILE_GROUP = {
  "reload_current_file_group", "A specific command to reload the current file or file group."
};

constexpr InteractiveCommandDoc<0> F3D_CMD_ADD_CURRENT_DIRECTORIES = { "add_current_directories",
  "A specific command to add all files from the current file or file group directories." };

constexpr InteractiveCommandDoc<1> F3D_CMD_TAKE_SCREENSHOT = { "take_screenshot",
  "A specific command to take a screenshot. If filename is not specified, rely on the "
  "--screenshot-filename CLI option.",
  { { "[filename]", "Optional filename for the screenshot" } } };

constexpr InteractiveCommandDoc<1> F3D_CMD_TAKE_MINIMAL_SCREENSHOT = { "take_minimal_screenshot",
  "A specific command to take a minimal screenshot. If filename is not specified, rely on the "
  "--screenshot-filename CLI option.",
  { { "[filename]", "Optional filename for the minimal screenshot" } } };

constexpr InteractiveCommandDoc<1> F3D_CMD_SET_HDRI = { "set_hdri",
  "A specific command to set and use an HDRI image. Take a HDRI file as an argument.",
  { { "[path/to/hdri]", "Path to the HDRI file" } } };

constexpr InteractiveCommandDoc<1> F3D_CMD_ADD_FILES_OR_SET_HDRI = { "add_files_or_set_hdri",
  "A specific command that will process each file and either set_hdri if the provided file uses a "
  "recognized HDR extension or add_files otherwise.",
  { { "[path/to/file1] [path/to/file2]", "One or more files to process" } } };

constexpr InteractiveCommandDoc<0> F3D_CMD_REMOVE_FILE_GROUPS = { "remove_file_groups",
  "A specific command to remove all files." };

constexpr InteractiveCommandDoc<2> F3D_CMD_ALIAS = { "alias",
  "A specific command to create an alias for a command.",
  { { "alias_name", "Name of the alias" }, { "command", "Command to associate with the alias" } } };

constexpr InteractiveCommandDoc<0> F3D_CMD_OPEN_FILE_DIALOG = { "open_file_dialog",
  "Show dialog to open a file." };

template<std::size_t N>
InteractiveCommand makeCommand(const InteractiveCommandDoc<N>& commandDoc,
  std::function<void(const std::vector<std::string>&)> callback)
{
  InteractiveCommand command;
  command.Name = std::string(commandDoc.Name);
  command.Documentation = std::string(commandDoc.Documentation);
  for (size_t i = 0; i < N; i++)
  {
    command.ArgsDocumentation.emplace_back(
      std::string(commandDoc.Arguments[i].first), std::string(commandDoc.Arguments[i].second));
  }
  command.Handler = callback;
  return command;
}

#endif
