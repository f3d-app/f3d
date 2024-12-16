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
  bool operator<(const interaction_bind_t& bind) const;

  /**
   * Operator to be able to store binds in maps and other structs
   * Compare modifier and interaction
   */
  bool operator==(const interaction_bind_t& bind) const;

  /**
   * Format this binding into a string
   * eg: "A", "Any+Question", "Shift+L".
   */
  std::string format() const;

  /**
   * Create and return an interaction bind from provided string
   */
  static interaction_bind_t parse(const std::string& str);
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
  virtual interactor& addCommand(
    const std::string& action, std::function<void(const std::vector<std::string>&)> callback) = 0;

  /**
   * Remove a command for provided action, does not do anything if it does not exists.
   */
  virtual interactor& removeCommand(const std::string& action) = 0;

  /**
   * Return a string vector containing all currently defined actions of commands
   */
  virtual std::vector<std::string> getCommandActions() const = 0;

  /**
   * Trigger provided command, see COMMANDS.md for details about supported
   * commands and syntax.
   *
   * If the command fails, it prints a debug log explaining why.
   *
   * Return true if the command succeeded, false otherwise.
   * Throw an interactor::command_runtime_exception if the command callback
   * throw an unrecognized exception. Note that default commands cannot throw such
   * an exception.
   */
  virtual bool triggerCommand(std::string_view command) = 0;
  ///@}

  ///@{ @name Bindings
  using documentation_callback_t = std::function<std::pair<std::string, std::string>()>;

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
    std::string group = {}, documentation_callback_t documentationCallback = nullptr) = 0;

  /**
   * See addBinding
   * Convenience method to add a single command for an interaction,
   * similar as `addBinding(bind, {command})`
   *
   * Adding command for an existing bind will throw a interactor::already_exists_exception.
   */
  virtual interactor& addBinding(const interaction_bind_t& bind, std::string command,
    std::string group = {}, documentation_callback_t documentationCallback = nullptr) = 0;

  /**
   * Convenience initializer list signature for add binding method
   */
  interactor& addBinding(const interaction_bind_t& bind, std::initializer_list<std::string> list,
    std::string group = {}, documentation_callback_t documentationCallback = nullptr)
  {
    return this->addBinding(
      bind, std::vector<std::string>(list), std::move(group), std::move(documentationCallback));
  }

  /**
   * Remove binding corresponding to provided bind.
   * Does not do anything if the provided bind does not exists.
   */
  virtual interactor& removeBinding(const interaction_bind_t& bind) = 0;

  /**
   * Return a vector of available bind groups, in order of addition
   */
  virtual std::vector<std::string> getBindGroups() const = 0;

  /**
   * Return a vector of bind for the specified group, in order of addition
   *
   * Getting binds for a group that does not exists will throw a does_not_exists_exception.
   */
  virtual std::vector<interaction_bind_t> getBindsForGroup(std::string group) const = 0;

  /**
   * Return a vector of all binds, in order of addition
   */
  virtual std::vector<interaction_bind_t> getBinds() const = 0;

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
  virtual std::pair<std::string, std::string> getBindingDocumentation(
    const interaction_bind_t& bind) const = 0;
  ///@}

  ///@{ @name Animation
  /**
   * Control the animation.
   */
  virtual void toggleAnimation() = 0;
  virtual void startAnimation() = 0;
  virtual void stopAnimation() = 0;
  virtual bool isPlayingAnimation() = 0;
  ///@}

  ///@{ @name Movement
  /**
   * Control if camera movements are enabled, which they are by default.
   */
  virtual void enableCameraMovement() = 0;
  virtual void disableCameraMovement() = 0;
  ///@}

  /**
   * Play a VTK interaction file.
   */
  virtual bool playInteraction(const std::string& file, double deltaTime = 1.0 / 30,
    std::function<void()> userCallBack = nullptr) = 0;

  /**
   * Start interaction and record it all in a VTK interaction file.
   */
  virtual bool recordInteraction(const std::string& file) = 0;

  /**
   * Start the interactor event loop.
   * The event loop will be triggered every deltaTime in seconds, and userCallBack will be called at
   * the start of the event loop
   */
  virtual void start(double deltaTime = 1.0 / 30, std::function<void()> userCallBack = nullptr) = 0;

  /**
   * Stop the interactor.
   */
  virtual void stop() = 0;

  /**
   * Request a render to be done on the next event loop
   * Safe to call in a multithreaded environment
   */
  virtual void requestRender() = 0;

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
inline interaction_bind_t interaction_bind_t::parse(const std::string& str)
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

    std::string modStr = str.substr(0, plusIt);
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
