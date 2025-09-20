#ifndef f3d_interactor_h
#define f3d_interactor_h

#include "exception.h"
#include "export.h"
#include "log.h"
#include "options.h"
#include "window.h"

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace f3d
{

struct interaction_bind_t
{
  /**
   * Enumeration of supported modifier combination, in binary.
   */
  enum class ModifierKeys : unsigned char
  {
    ANY = 0x80,      // 10000000
    NONE = 0x0,      // 00000000
    CTRL = 0x1,      // 00000001
    SHIFT = 0x2,     // 00000010
    CTRL_SHIFT = 0x3 // 00000011
  };

  ModifierKeys mod = ModifierKeys::NONE;
  std::string inter;

  /**
   * Operator to be able to store binds in maps and other structs
   * Compare modifier and interaction
   */
  [[nodiscard]] bool operator<(const interaction_bind_t& bind) const;

  /**
   * Operator to be able to store binds in maps and other structs
   * Compare modifier and interaction
   */
  [[nodiscard]] bool operator==(const interaction_bind_t& bind) const;

  /**
   * Format this binding into a string
   * eg: "A", "Any+Question", "Shift+L".
   */
  [[nodiscard]] std::string format() const;

  /**
   * Create and return an interaction bind from provided string
   */
  [[nodiscard]] static interaction_bind_t parse(std::string_view str);
};

/**
 * @class   interactor
 * @brief   Class used to control interaction and animation
 *
 * A class to control interaction with the window as well as animation.
 * It also provide a timer callback if needed and can record/play interaction file.
 */
class F3D_EXPORT interactor
{
public:
  ///@{ @name Command
  using command_documentation_t = std::pair<std::string, std::string>;

  /**
   * Remove all existing commands and add all default commands,
   * see COMMANDS.md for details.
   */
  virtual interactor& initCommands() = 0;

  /**
   * Use this method to add a command to be called using triggerCommand.
   * Adding a command with an already existing action throw a
   * interactor::already_exists_exception.
   * Considering namespacing dedicated action to avoid conflicts with default action,
   * eg: `my_app::action`
   */
  virtual interactor& addCommand(const std::string& action,
    std::function<void(const std::vector<std::string>&)> callback,
    std::optional<command_documentation_t> doc = std::nullopt,
    std::function<std::vector<std::string>(const std::vector<std::string>&)> completionCallback =
      nullptr) = 0;

  /**
   * Remove a command for provided action, does not do anything if it does not exists.
   */
  virtual interactor& removeCommand(const std::string& action) = 0;

  /**
   * Return a string vector containing all currently defined actions of commands
   */
  [[nodiscard]] virtual std::vector<std::string> getCommandActions() const = 0;

  /**
   * Trigger provided command, see COMMANDS.md for details about supported
   * commands and syntax.
   *
   * If the command fails, it prints a debug log explaining why.
   *
   * When keepComments argument is true, comments are supported with `#`, any characters after are
   * ignored otherwise '#' and any characters after will be handled as standard character
   *
   * Return true if the command succeeded, false otherwise.
   * Throw an interactor::command_runtime_exception if the command callback
   * throw an unrecognized exception.
   * Note that default commands will never throw this exception, but adding commands
   * without exception catching may trigger this behavior.
   */
  virtual bool triggerCommand(std::string_view command, bool keepComments = true) = 0;
  ///@}

  ///@{ @name Bindings
  using documentation_callback_t = std::function<std::pair<std::string, std::string>()>;

  /**
   * Enumeration of binding types.
   * Duplication present in vtkext/private/module/vtkF3DUIActor.h.
   */
  enum class BindingType : std::uint8_t
  {
    CYCLIC = 0,
    NUMERICAL = 1,
    TOGGLE = 2,
    OTHER = 3,
  };

  /**
   * Remove all existing interaction commands and add all default bindings
   * see INTERACTIONS.md for details.
   */
  virtual interactor& initBindings() = 0;

  /**
   * Use this method to add binding, in order to trigger commands for a specific bind
   *
   * Bind modifiers is a binary flag from the dedicated enum that represent KeyModifiers.
   * Bind interaction can be a pressed key symbol, eg: "C",
   * or a dedicated key symbol for special keys:
   * "Left", "Right", "Up", "Down", "Space", "Enter", "Escape", "Question".
   *
   * group is an optional arg to group bindings together for better display of the documentation.
   * Groups are kept in order of addition when recovered using `getBindGroups`.
   * Bindings are kept in order of addition when recovered using `getBindsForGroup`.
   *
   * documentationCallback is an optional function that returns a pair of string,
   * the first is the doc itself, the second is the current value as a string, if any.
   * Use `getBindingDocumentation` to access this doc.
   *
   * type is an optional type of binding to provide, it can be used for presenting the
   * binding in a coherent way in logs and cheatsheet.
   *
   * When the corresponding bind happens, the provided commands will be triggered using
   * triggerCommand. Considering checking if an interaction exists or removing it before adding it
   * to avoid potential conflicts.
   *
   * ANY modifier interactions will only be triggered if no other interaction bind with modifier
   * is found.
   *
   * Adding commands for an existing bind will throw a interactor::already_exists_exception.
   */
  virtual interactor& addBinding(const interaction_bind_t& bind, std::vector<std::string> commands,
    std::string group = {}, documentation_callback_t documentationCallback = nullptr,
    BindingType type = BindingType::OTHER) = 0;

  /**
   * See addBinding
   * Convenience method to add a single command for an interaction,
   * similar as `addBinding(bind, {command})`
   *
   * Adding command for an existing bind will throw a interactor::already_exists_exception.
   */
  virtual interactor& addBinding(const interaction_bind_t& bind, std::string command,
    std::string group = {}, documentation_callback_t documentationCallback = nullptr,
    BindingType type = BindingType::OTHER) = 0;

  /**
   * Convenience initializer list signature for add binding method
   */
  interactor& addBinding(const interaction_bind_t& bind, std::initializer_list<std::string> list,
    std::string group = {}, documentation_callback_t documentationCallback = nullptr,
    BindingType type = BindingType::OTHER)
  {
    return this->addBinding(bind, std::vector<std::string>(list), std::move(group),
      std::move(documentationCallback), type);
  }

  /**
   * Remove binding corresponding to provided bind.
   * Does not do anything if the provided bind does not exists.
   */
  virtual interactor& removeBinding(const interaction_bind_t& bind) = 0;

  /**
   * Return a vector of available bind groups, in order of addition
   */
  [[nodiscard]] virtual std::vector<std::string> getBindGroups() const = 0;

  /**
   * Return a vector of bind for the specified group, in order of addition
   *
   * Getting binds for a group that does not exists will throw a does_not_exists_exception.
   */
  [[nodiscard]] virtual std::vector<interaction_bind_t> getBindsForGroup(
    std::string group) const = 0;

  /**
   * Return a vector of all binds, in order of addition
   */
  [[nodiscard]] virtual std::vector<interaction_bind_t> getBinds() const = 0;

  /**
   * Get a pair of string documenting a binding.
   * The first string is the documentation of the binding,
   * eg: "Toggle anti aliasing", "Print scene descr to terminal", "Decrease light intensity"
   * The second string is the current value of the binding,
   * eg: "OFF", "" if there is no value or "1.12".
   * If a binding was not documented on addition, the provided strings will be empty.
   * The possible string can depends on the bindings but boolean value are expected to be
   * "ON", "OFF", "N/A" (for optional values).
   *
   * Getting documentation for a bind that does not exists will throw a does_not_exists_exception.
   */
  [[nodiscard]] virtual std::pair<std::string, std::string> getBindingDocumentation(
    const interaction_bind_t& bind) const = 0;
  ///@}

  /**
   * Get the type of a binding.
   *
   * Getting type for a bind that does not exists will throw a does_not_exists_exception.
   */
  [[nodiscard]] virtual BindingType getBindingType(const interaction_bind_t& bind) const = 0;
  ///@}

  ///@{ @name Animation
  /**
   * Control the animation.
   */
  virtual interactor& toggleAnimation() = 0;
  virtual interactor& startAnimation() = 0;
  virtual interactor& stopAnimation() = 0;
  [[nodiscard]] virtual bool isPlayingAnimation() = 0;
  ///@}

  ///@{ @name Movement
  /**
   * Control if camera movements are enabled, which they are by default.
   */
  virtual interactor& enableCameraMovement() = 0;
  virtual interactor& disableCameraMovement() = 0;
  ///@}

  ///@{ @name Forwarding input events
  /**
   * Enumeration of supported mouse buttons.
   */
  enum class MouseButton : unsigned char
  {
    LEFT,
    RIGHT,
    MIDDLE
  };

  /**
   * Enumeration of supported mouse wheel directions.
   */
  enum class WheelDirection : unsigned char
  {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
  };

  /**
   * Enumeration of supported input actions.
   */
  enum class InputAction : unsigned char
  {
    PRESS,
    RELEASE
  };

  /**
   * Enumeration of supported input modifiers.
   */
  enum class InputModifier : unsigned char
  {
    NONE,
    CTRL,
    SHIFT,
    CTRL_SHIFT
  };

  /**
   * Trigger a modifier update.
   * This will update the internal modifier state of the interactor to match the provided one.
   */
  virtual interactor& triggerModUpdate(InputModifier mod) = 0;

  /**
   * Trigger a mouse button event.
   * This will trigger the corresponding mouse button press or release event.
   */
  virtual interactor& triggerMouseButton(InputAction action, MouseButton button) = 0;

  /**
   * Trigger a mouse new position event.
   * Positions are in window coordinates, with (0, 0) being the top-left corner.
   * The coordinates are expressed in pixels.
   */
  virtual interactor& triggerMousePosition(double xpos, double ypos) = 0;

  /**
   * Trigger a mouse wheel event.
   * At the moment, only vertical wheel events are observed, but it can change in the future.
   */
  virtual interactor& triggerMouseWheel(WheelDirection direction) = 0;

  /**
   * Trigger a keyboard key event.
   * This is based on X11 key symbols, it's hard to list all of them, but here are a few:
   * - "A", "B", "C", ..., "Z" for letters
   * - "0", "1", "2", ..., "9" for numbers
   * - "Left", "Right", "Up", "Down" for arrow keys
   * - "Space", "Return", "Escape", "Tab", "BackSpace" for common keys
   * - "F1", "F2", ..., "F25" for function keys
   * - "KP_0", "KP_1", ..., "KP_9" for numpad keys
   * @note
   * It's possible to run F3D application in verbose mode and press keys to print their symbols.
   */
  virtual interactor& triggerKeyboardKey(InputAction action, std::string_view keySym) = 0;

  /**
   * Trigger a text character input event.
   * This will trigger the corresponding character input event, with the codepoint being the Unicode
   * codepoint of the character.
   * It's used for text input, like when typing in a the console input field.
   */
  virtual interactor& triggerTextCharacter(unsigned int codepoint) = 0;
  ///@}

  /**
   * Play a VTK interaction file.
   * Provided file path is used as is and file existence will be checked.
   * The event loop will be triggered every deltaTime in seconds, and userCallBack will be called at
   * the start of the event loop
   */
  virtual bool playInteraction(const std::filesystem::path& file, double deltaTime = 1.0 / 30,
    std::function<void()> userCallBack = nullptr) = 0;

  /**
   * Start interaction and record it all in a VTK interaction file.
   * Provided file path will be used as is and the parent directories of the file will be created
   */
  virtual bool recordInteraction(const std::filesystem::path& file) = 0;

  /**
   * Start the interactor event loop.
   * The event loop will be triggered every deltaTime in seconds, and userCallBack will be called at
   * the start of the event loop
   */
  virtual interactor& start(
    double deltaTime = 1.0 / 30, std::function<void()> userCallBack = nullptr) = 0;

  /**
   * Stop the interactor.
   */
  virtual interactor& stop() = 0;

  /**
   * Request a render to be done on the next event loop
   * Safe to call in a multithreaded environment
   */
  virtual interactor& requestRender() = 0;

  /**
   * An exception that can be thrown by the interactor
   * when adding something that already exists internally
   */
  struct already_exists_exception : public exception
  {
    explicit already_exists_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by the interactor
   * when looking for something that does not exists
   */
  struct does_not_exists_exception : public exception
  {
    explicit does_not_exists_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by interactor::triggerCommand
   * when a command callback throw an exception
   */
  struct command_runtime_exception : public exception
  {
    explicit command_runtime_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown by command callbacks
   * when the arguments of the callback are incorrect.
   * This exception is caught by triggerCommand and logged.
   */
  struct invalid_args_exception : public exception
  {
    explicit invalid_args_exception(const std::string& what = "")
      : exception(what)
    {
    }
  };

protected:
  //! @cond
  interactor() = default;
  virtual ~interactor() = default;
  interactor(const interactor& opt) = delete;
  interactor(interactor&& opt) = delete;
  interactor& operator=(const interactor& opt) = delete;
  interactor& operator=(interactor&& opt) = delete;
  //! @endcond
};

//----------------------------------------------------------------------------
inline bool interaction_bind_t::operator<(const interaction_bind_t& bind) const
{
  return this->mod < bind.mod || (this->mod == bind.mod && this->inter < bind.inter);
}

//----------------------------------------------------------------------------
inline bool interaction_bind_t::operator==(const interaction_bind_t& bind) const
{
  return this->mod == bind.mod && this->inter == bind.inter;
}

//----------------------------------------------------------------------------
inline std::string interaction_bind_t::format() const
{
  switch (this->mod)
  {
    case ModifierKeys::CTRL_SHIFT:
      return "Ctrl+Shift+" + this->inter;
    case ModifierKeys::CTRL:
      return "Ctrl+" + this->inter;
    case ModifierKeys::SHIFT:
      return "Shift+" + this->inter;
    case ModifierKeys::ANY:
      return "Any+" + this->inter;
    default:
      // No need to check for NONE
      return this->inter;
  }
}

//----------------------------------------------------------------------------
inline interaction_bind_t interaction_bind_t::parse(std::string_view str)
{
  interaction_bind_t bind;
  auto plusIt = str.find_last_of('+');
  if (plusIt == std::string::npos)
  {
    bind.inter = str;
  }
  else
  {
    bind.inter = str.substr(plusIt + 1);

    std::string_view modStr = str.substr(0, plusIt);
    if (modStr == "Ctrl+Shift")
    {
      bind.mod = ModifierKeys::CTRL_SHIFT;
    }
    else if (modStr == "Shift")
    {
      bind.mod = ModifierKeys::SHIFT;
    }
    else if (modStr == "Ctrl")
    {
      bind.mod = ModifierKeys::CTRL;
    }
    else if (modStr == "Any")
    {
      bind.mod = ModifierKeys::ANY;
    }
    else if (modStr == "None")
    {
      bind.mod = ModifierKeys::NONE;
    }
    else
    {
      f3d::log::warn("Invalid modifier: ", modStr, ", ignoring modifier");
    }
  }
  return bind;
}
}

#endif
