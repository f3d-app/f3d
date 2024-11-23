#ifndef f3d_interactor_h
#define f3d_interactor_h

#include "exception.h"
#include "export.h"
#include "options.h"
#include "window.h"

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace f3d
{
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

  /**
   * Remove all existing interaction commands and add all default bindings
   * see INTERACTIONS.md for details.
   */
  virtual interactor& initBindings() = 0;

  /**
   * Use this method to add binding, in order to trigger commands for a specified interaction and
   * modifiers flag.
   *
   * interaction can be a pressed key symbol, eg: "C",
   * or a dedicated key symbol for special keys:
   * "Left", "Right", "Up", "Down", "Space", "Enter", "Escape", "Question".
   *
   * modifiers is a binary flag from the dedicated enum that represent KeyModifiers.
   *
   * documentationCallback is an optional function that returns a pair of string,
   * the first is the doc itself, the second is the current value as a string, if any.
   * Use `getBindingsDocumentation` to access this doc.
   *
   * When the corresponding interaction and modifiers happens, the provided commands will be
   * triggered using triggerCommand.
   * Considering checking if an interaction exists or removing it before adding it to avoid
   * potential conflicts.
   *
   * ANY modifier interactions will only be triggered if no other interaction bind with modifier
   * is found.
   *
   * Adding commands for an existing combination of interaction and modifier will throw a
   * interactor::already_exists_exception.
   */
  virtual interactor& addBinding(const std::string& interaction, ModifierKeys modifiers,
    std::vector<std::string> commands, std::string group = std::string(),
    std::function<std::pair<std::string, std::string>()> documentationCallback = nullptr) = 0;

  /**
   * See addBinding
   * Convenience method to add a single command for an interaction, similar as
   * addBinding(interaction, modifiers, {command})
   *
   * Adding command for an existing combination of interaction and modifier will throw a
   * interactor::already_exists_exception.
   */
  virtual interactor& addBinding(const std::string& interaction, ModifierKeys modifiers,
    std::string command, std::string group = std::string(),
    std::function<std::pair<std::string, std::string>()> documentationCallback = nullptr) = 0;

  /**
   * Convenience initializer list signature for add binding method
   */
  interactor& addBinding(const std::string& interaction, ModifierKeys modifiers,
    std::initializer_list<std::string> list, std::string group = std::string(),
    std::function<std::pair<std::string, std::string>()> documentationCallback = nullptr)
  {
    return this->addBinding(
      interaction, modifiers, std::vector<std::string>(list), group, documentationCallback);
  }

  /**
   * Remove binding corresponding to provided interaction and modifiers
   */
  virtual interactor& removeBinding(std::string interaction, ModifierKeys modifiers) = 0;

  virtual std::vector<std::string> getBindingGroups() const = 0;
  virtual std::vector<std::pair<std::string, ModifierKeys>> getBindingsForGroup(std::string group) const = 0;

  /**
   * Return a string vector of all currently defined bind interactions
   */
//  virtual std::vector<std::pair<std::string, ModifierKeys>> getBindingInteractions() const = 0;

  virtual std::pair<std::string, std::string> getBindingDocumentation(std::string interaction, ModifierKeys modifiers) const = 0;

  /**
   * Get a structure of strings documenting bindings.
   * This returns a vectors of tuple of strings, with one entry in the vector by documented binding
   * added either by default or by `addBinding`.
   * This is the method used to print the cheatsheet.
   *
   * The first string in the tuple is the binding interaction itself, as a string,
   * eg: "A", "ANY+Question", "SHIFT+L".
   * The second scring in the tuple is the documentation of the binding,
   * eg: "Toggle anti aliasing", "Print scene descr to terminal", "Decrease light intensity"
   * The third string of the tuple is the current value of the binding,
   * eg: "OFF", "" if there is no value or "1.12".
   * The possible string can depends on the bindings but boolean value are expected to be
   * "ON", "OFF", "NO SET" (for optional values).
   */
//  virtual std::vector<std::tuple<std::string, std::string, std::string>> getBindingsDocumentation()
//    const = 0;
  ///@}

  /**
   * Use this method to create your own timer callback. You callback will be called once every time
   * ms. Return an id to use in removeTimeCallBack.
   */
  virtual unsigned long createTimerCallBack(double time, std::function<void()> callBack) = 0;

  /**
   * Remove a previously created timer callback using the id.
   */
  virtual void removeTimerCallBack(unsigned long id) = 0;

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
  virtual bool playInteraction(const std::string& file) = 0;

  /**
   * Start interaction and record it all in a VTK interaction file.
   */
  virtual bool recordInteraction(const std::string& file) = 0;

  /**
   * Start the interactor.
   */
  virtual void start() = 0;

  /**
   * Stop the interactor.
   */
  virtual void stop() = 0;

  /**
   * An exception that can be thrown by the interactor
   * when adding something that already exists internally
   */
  struct already_exists_exception : public exception
  {
    explicit already_exists_exception(const std::string& what = "");
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
}

#endif
