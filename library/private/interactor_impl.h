/**
 * @class   interactor_impl
 * @brief   A concrete implementation of interactor
 *
 * A concrete implementation of interactor that hides the private API
 * See interactor.h for the class documentation
 */

#ifndef f3d_interactor_impl_h
#define f3d_interactor_impl_h

#include "interactor.h"

#include <memory>

class vtkInteractorObserver;
class vtkImporter;
namespace f3d
{
class options;

namespace detail
{
class scene_impl;
class window_impl;
class animationManager;

class interactor_impl : public interactor
{
public:
  ///@{
  /**
   * Documented public API
   */
  interactor_impl(options& options, window_impl& window, scene_impl& scene);
  ~interactor_impl() override;

  interactor& initCommands() override;
  interactor& addCommand(
    std::string action, std::function<void(const std::vector<std::string>&)> callback) override;
  interactor& removeCommand(const std::string& action) override;
  std::vector<std::string> getCommandActions() const override;
  bool triggerCommand(std::string_view command) override;

  interactor& initBindings() override;
  interactor& addBinding(const interaction_bind_t& bind, std::vector<std::string> commands,
    std::string group = std::string(),
    documentation_callback_t documentationCallback = nullptr) override;
  interactor& addBinding(const interaction_bind_t& bind, std::string command,
    std::string group = std::string(),
    documentation_callback_t documentationCallback = nullptr) override;
  interactor& removeBinding(const interaction_bind_t& bind) override;
  std::vector<std::string> getBindGroups() const override;
  std::vector<interaction_bind_t> getBindsForGroup(std::string group) const override;
  std::vector<interaction_bind_t> getBinds() const override;
  std::pair<std::string, std::string> getBindingDocumentation(
    const interaction_bind_t& bind) const override;

  interactor& triggerModUpdate(InputModifier mod) override;
  interactor& triggerMouseButton(InputAction action, MouseButton button) override;
  interactor& triggerMousePosition(double xpos, double ypos) override;
  interactor& triggerMouseWheel(WheelDirection direction) override;
  interactor& triggerKeyboardKey(InputAction action, std::string_view keySym) override;
  interactor& triggerTextCharacter(unsigned int codepoint) override;

  interactor& toggleAnimation() override;
  interactor& startAnimation() override;
  interactor& stopAnimation() override;
  bool isPlayingAnimation() override;

  interactor& enableCameraMovement() override;
  interactor& disableCameraMovement() override;

  bool playInteraction(const std::filesystem::path& file, double deltaTime,
    std::function<void()> userCallBack) override;
  bool recordInteraction(const std::filesystem::path& file) override;

  interactor& start(double deltaTime, std::function<void()> userCallBack) override;
  interactor& stop() override;
  interactor& requestRender() override;
  ///@}

  /**
   * Implementation only API.
   * Set the internal AnimationManager to be used by the interactor
   */
  void SetAnimationManager(animationManager* manager);

  /**
   * Implementation only API.
   * An utility method to set internal VTK interactor on a vtkInteractorObserver object.
   */
  void SetInteractorOn(vtkInteractorObserver* observer);

  /**
   * Implementation only API.
   * Initialize the animation manager using interactor objects.
   * This is called by the scene after add a file.
   */
  void InitializeAnimation(vtkImporter* importer);

  /**
   * Implementation only API
   * Forward to vtkF3DInteractorStyle so that
   * it update the renderer as needed, especially
   * the camera clipping range.
   */
  void UpdateRendererAfterInteraction();

  /**
   * Implementation only API.
   * Expose the method to reset transformed up vector.
   * This is called by the scene after initializing the up vector.
   */
  void ResetTemporaryUp();

  /**
   * Event loop being called automatically once the interactor is started
   * First call the EventLoopUserCallBack, then call render if requested.
   */
  void EventLoop();

  /**
   * Set a command to be run on the next event loop
   */
  void SetCommandBuffer(const char* command);

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif
