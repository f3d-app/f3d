/**
 * @class   animation_impl
 * @brief   A concrete implementation of animation
 *
 * A concrete implementation of animation that hides the private API
 * See animation.h for the class documentation
 */

#ifndef f3d_animation_impl_h
#define f3d_animation_impl_h

#include "animation.h"
#include "window_impl.h"

#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <chrono>
#include <optional>
#include <set>

namespace f3d
{

namespace detail
{

class animation_impl : public animation
{
public:
  ///@{
  /**
   * Documented public API
   */
  animation_impl(options& options, window_impl& window);
  ~animation_impl() override;
  animation& loadTime(double timeValue) override;
  std::pair<double, double> getTimeRange() override;
  std::vector<double> getKeyFrames() override;
  unsigned int count() const override;
  std::string getName(std::optional<int> index = std::nullopt) const override;
  std::vector<std::string> getNames() const override;
  ///@}

  /**
   * Implementation only API
   * Set the interactor to use in the animation_manager, should be set before initializing if any
   */
  void SetInteractor(interactor_impl* interactor);

  /**
   * Implementation only API
   * Set the importer to use in the animation_manager, must be set before initializing
   */
  void SetImporter(vtkF3DMetaImporter* importer);

  /**
   * Implementation only API
   * Set animation direction,
   * Only following values are correct :
   * 1 for forward animation
   * -1 for backward animation
   */
  void SetAnimationDirection(int direction);

  /**
   * Implementation only API
   * Initialize the animation, required before playing the animation.
   * Can be used to reset animation to the initial state.
   * Importer must be set before use.
   * Interactor should be set before use if any.
   * Also start the animation when using autoplay option
   */
  void Initialize();

  /**
   * Implementation only API
   * Reset the animation to a no-animation state.
   */
  void Reset();

  /**
   * Implementation only API
   * Start/Stop playing the animation
   * Direction must always be equal to 1 (forward) or -1 (backward)
   */
  void ToggleAnimation();
  void StartAnimation();
  void StopAnimation();

  /**
   * Implementation only API
   * Cycle onto and play the next available animation
   * This modifies the scene.animation.index option
   */
  void CycleAnimation();

  /**
   * Implementation only API
   * Return animation direction
   * 1 for forward animation
   * -1 for backward animation
   */
  int GetAnimationDirection() const;

  /**
   * Implementation only API
   * Return true if the animation is being played
   */
  bool IsPlaying() const;

  /**
   * Implementation only API
   * Return the current animation time in seconds
   */
  double GetCurrentTime() const;

  /**
   * Implementation only API
   *Set the animation in delta time in seconds
   */
  void SetDeltaTime(double deltaTime);

  /**
   * Implementation only API
   * Advance animationTime of deltaTime and call loadAtTime accordingly
   * Do nothing if IsPlaying is false
   */
  void Tick();

  /**
   * Implementation only API
   * Load animation at provided time value
   */
  bool LoadAtTime(double timeValue);

  /**
   * Implementation only API
   * Load animation at provided frame value
   * When relative is false frame -1 is equal to last frame
   */
  void JumpToFrame(int frame, bool relative);

  /**
   * Implementation only API
   * Load animation at a specific key frame
   * When relative is false key frame -1 is equal to last key frame
   */
  void JumpToKeyFrame(int keyFrame, bool relative);

  /**
   * Implementation only API
   * Load animation at provided time value and render
   * When relative is true, time is added to the current animation time
   * When relative is false, a negative time is counted from the end of the animation
   */
  void JumpToTime(double timeValue, bool relative);

  /**
   * Implementation only API
   * Update the dynamic options value to trigger cheatsheet update if needed.
   */
  void UpdateDynamicOptions();

private:
  /**
   * Prepare time range and internal members for animation indices from options
   * Return early if already prepared for the current subset of animation in the options
   */
  void PrepareForAnimationIndices();

  /**
   * Push the current animation's time range and name to the UI actor.
   */
  void PushAnimationProgress();

  /**
   * Internal setter for Autoplay.
   */
  void SetAutoplay(bool enable);

  /**
   * Internal setter for SpeedFactor.
   */
  void SetSpeedFactor(double speedFactor);

  /**
   * Helper method to call the homonymous method from vtkF3DRenderer.
   */
  void SetCheatSheetConfigured(bool configured);

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
}

#endif
