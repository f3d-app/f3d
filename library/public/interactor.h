#ifndef f3d_interactor_h
#define f3d_interactor_h

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
  /**
   * Use this method to specify your own drop files callback, with the expected API:
   * \code
   * bool callBack(std::vector<std::string> files)
   * \endcode
   * files being a vector of string containing paths to dropped files.
   * Your callBack should return true if the event was handled, false if you want standard
   * interactor behavior instead.
   */
  virtual interactor& setDropFilesCallBack(
    std::function<bool(std::vector<std::string>)> callBack) = 0;

  ///@{ @name Command
  /**
   * Use this method to add a callback into the command map
   * to be called using triggerCommand.
   * Adding a commandCallback with an existing action replaces it.
   */
  virtual interactor& addCommandCallback(
    std::string action, std::function<bool(const std::vector<std::string>&)> callback) = 0;

  /**
   * Remove a command callback for provided action
   */
  virtual interactor& removeCommandCallback(const std::string& action) = 0;

  /**
   * Trigger provided command, see COMMAND.md for more information
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

  struct InteractionBind
  {
    std::string Interaction;
    ModifierKeys Modifiers;

    // TODO use decltype in cxx instead?
    bool operator<(const f3d::interactor::InteractionBind& bind) const
    {
      return this->Interaction < bind.Interaction ||
        (this->Interaction == bind.Interaction && this->Modifiers < bind.Modifiers);
    }
  };

  /**
   * Use this method to specify your own interaction commands for a specified interaction and
   * modifiers flag.
   *
   * InteractionBind.Interaction can be a pressed key symbol, eg: "C",
   * or a dedicated key symbol for special keys:
   * "Left", "Right", "Up", "Down", "Space", "Enter", "Escape", "Question".
   *
   * InteractionBind.Modifiers is a binary flag from the dedicated enum that represent KeyModifiers.
   *
   * Adding commands for an existing InteractionBind will replace it.
   *
   * When the corresponding interaction and modifiers happens, the provided commands will be
   * triggered using triggerCommand.
   */
  virtual interactor& addInteractionCommands(
    InteractionBind bind, const std::vector<std::string>& commands) = 0;

  /**
   * See addInteractionCommands
   * Convenience method to add a single command for an interaction, similar as
   * addInteractionCommands(bind, {command})
   */
  virtual interactor& addInteractionCommand(InteractionBind bind, const std::string& command) = 0;

  /**
   * Remove interaction commands corresponding to provided interaction and modifiers
   */
  virtual interactor& removeInteractionCommands(InteractionBind bind) = 0;
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
