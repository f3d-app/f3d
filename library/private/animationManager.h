/**
 * @class   animationManager
 * @brief   A private class managing animation
 */

#ifndef f3d_animationManager_h
#define f3d_animationManager_h

#include <vtkNew.h>
#include <vtkProgressBarWidget.h>
#include <vtkSmartPointer.h>

#include <chrono>
#include <optional>
#include <set>

class vtkF3DRenderer;
class vtkImporter;
class vtkRenderWindow;

namespace f3d
{
class options;

namespace detail
{
class interactor_impl;
class window_impl;

class animationManager
{
public:
  animationManager(options& options, window_impl& window);
  ~animationManager() = default;

  /**
   * Set the interactor to use in the animation_manager, should be set before initializing if any
   */
  void SetInteractor(interactor_impl* interactor);

  /**
   * Set the importer to use in the animation_manager, must be set before initializing
   */
  void SetImporter(vtkImporter* importer);

  /**
   * Initialize the animation manager, required before playing the animation.
   * Can be used to reset animation to the initial state.
   * Importer must be set before use.
   * Interactor should be set before use if any.
   * Also start the animation when using autoplay option
   */
  void Initialize();

  /**
   * Start/Stop playing the animation
   */
  void ToggleAnimation();
  void StartAnimation();
  void StopAnimation();

  /**
   * Cycle onto and play the next available animation
   * This modifies the scene.animation.index option
   */
  void CycleAnimation();

  /**
   * Return the current animation name if any
   * Can be called before initialization safely
   */
  std::string GetAnimationName();

  /**
   * Return true if the animation manager is playing the animation
   */
  bool IsPlaying() const
  {
    return Playing;
  }

  /**
   * Set the animation in delta time in seconds
   */
  void SetDeltaTime(double deltaTime);

  /**
   * Advance animationTime of DeltaTime and call loadAtTime accordingly
   * Do nothing if IsPlaying is false
   */
  void Tick();

  /**
   * Load animation at provided time value
   */
  bool LoadAtTime(double timeValue);

  /**
   * Return a pair containing the current time range values
   */
  std::pair<double, double> GetTimeRange();

  /**
   * Get the number of available animations
   */
  unsigned int GetNumberOfAvailableAnimations() const;

  animationManager(animationManager const&) = delete;
  void operator=(animationManager const&) = delete;

private:
  /**
   * Prepare time range and internal members for animation indices from options
   * Return early if already prepared for the current subset of animation in the options
   */
  void PrepareForAnimationIndices();

  options& Options;
  window_impl& Window;
  vtkImporter* Importer = nullptr;
  interactor_impl* Interactor = nullptr;

  int AvailAnimations = 0;

  std::optional<std::vector<int>> PreparedAnimationIndices;
  double TimeRange[2] = { 0.0, 0.0 };
  bool Playing = false;
  double CurrentTime = 0;
  double DeltaTime = 0;
  bool CurrentTimeSet = false;

  vtkSmartPointer<vtkProgressBarWidget> ProgressWidget;
};
}
}
#endif
