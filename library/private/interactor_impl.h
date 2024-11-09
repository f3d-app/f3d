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

  interactor& setDropFilesCallBack(std::function<bool(std::vector<std::string>)> callBack) override;

  interactor& addCommandCallback(
    std::string action, std::function<bool(const std::vector<std::string>&)> callback) override;
  interactor& removeCommandCallback(const std::string& action) override;
  bool triggerCommand(std::string_view command) override;
  interactor& addInteractionCommands(InteractionBind bind, const std::vector<std::string>& command) override;
  interactor& addInteractionCommand(InteractionBind bind, const std::string& command) override;
  interactor& removeInteractionCommands(InteractionBind bind) override;

  unsigned long createTimerCallBack(double time, std::function<void()> callBack) override;
  void removeTimerCallBack(unsigned long id) override;

  void toggleAnimation() override;
  void startAnimation() override;
  void stopAnimation() override;
  bool isPlayingAnimation() override;

  void enableCameraMovement() override;
  void disableCameraMovement() override;

  bool playInteraction(const std::string& file) override;
  bool recordInteraction(const std::string& file) override;

  void start() override;
  void stop() override;
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

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif
