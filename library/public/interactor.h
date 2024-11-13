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
   * Create and replace all default command callbacks, see COMMANDS.md
   * for details.
   */
  virtual interactor& createDefaultCommandCallbacks() = 0;

  /**
   * Use this method to add a callback into the command map
   * to be called using triggerCommand.
   * Adding a commandCallback with an already existing action throw a
   * interactor::already_exists_exception.
   */
  virtual interactor& addCommandCallback(
    const std::string& action, std::function<bool(const std::vector<std::string>&)> callback) = 0;

  /**
   * Similar as addCommandCallback but either add or replace an already existing action
   * instead of throwing.
   */
  virtual interactor& replaceCommandCallback(
    std::string action, std::function<bool(const std::vector<std::string>&)> callback) = 0;

  /**
   * Remove a command callback for provided action
   */
  virtual interactor& removeCommandCallback(const std::string& action) = 0;

  /**
   * Remove a all existing command callbacks
   */
  virtual interactor& removeAllCommandCallbacks() = 0;

  /**
   * Trigger provided command, see COMMANDS.md for details about supported
   * commands and syntax.
   */
  virtual bool triggerCommand(std::string_view command) = 0;
  ///@}

  ///@{ @name Interaction Commands
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
   * Create and replace all commands for default interactions, see INTERACTIONS.md
   * for details.
   */
  virtual interactor& createDefaultInteractionsCommands() = 0;

  /**
   * Use this method to specify your own interaction commands for a specified interaction and
   * modifiers flag.
   *
   * interaction can be a pressed key symbol, eg: "C",
   * or a dedicated key symbol for special keys:
   * "Left", "Right", "Up", "Down", "Space", "Enter", "Escape", "Question".
   *
   * modifiers is a binary flag from the dedicated enum that represent KeyModifiers.
   *
   * When the corresponding interaction and modifiers happens, the provided commands will be
   * triggered using triggerCommand.
   * ANY modifier interactions will only be triggered if no other interaction bind with modifier
   * is found.
   *
   * Adding commands for an existing combination of interaction and modifier will throw a
   * interactor::already_exists_exception.
   */
  virtual interactor& addInteractionCommands(
    const std::string& interaction, ModifierKeys modifiers, std::vector<std::string> commands) = 0;

  /**
   * See addInteractionCommands
   * Convenience method to add a single command for an interaction, similar as
   * addInteractionCommands(interaction, modifiers, {command})
   *
   * Adding command for an existing combination of interaction and modifier will throw a
   * interactor::already_exists_exception.
   */
  virtual interactor& addInteractionCommand(
    const std::string& interaction, ModifierKeys modifiers, std::string command) = 0;

  /**
   * Similar to addInteractionCommands but either add or replace commands for an existing
   * combination instead of throwing.
   */
  virtual interactor& replaceInteractionCommands(
    std::string interaction, ModifierKeys modifiers, std::vector<std::string> commands) = 0;

  /**
   * See replaceInteractionCommands
   * Convenience method to either add or replace a single command for an interaction, similar as
   * replaceInteractionCommands(interaction, modifiers, {command})
   */
  virtual interactor& replaceInteractionCommand(
    std::string interaction, ModifierKeys modifiers, std::string command) = 0;

  /**
   * Remove interaction commands corresponding to provided interaction and modifiers
   */
  virtual interactor& removeInteractionCommands(
    std::string interaction, ModifierKeys modifiers) = 0;

  /**
   * Remove all commands for all interactions
   */
  virtual interactor& removeAllInteractionsCommands() = 0;
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
   * Get a structure of strings describing default interactions.
   */
  static const std::vector<std::pair<std::string, std::string>>& getDefaultInteractionsInfo();

  /**
   * An exception that can be thrown by the interactor
   * when adding something that already exists internally
   */
  struct already_exists_exception : public exception
  {
    explicit already_exists_exception(const std::string& what = "");
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
