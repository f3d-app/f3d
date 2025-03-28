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
  animationManager(const options& options, window_impl& window);
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
   */
  void CycleAnimation();

  /**
   * Enable only the current animation
   */
  void EnableOnlyCurrentAnimation();

  /**
   * Get the current animation index
   */
  int GetAnimationIndex();

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

  animationManager(animationManager const&) = delete;
  void operator=(animationManager const&) = delete;

  /**
   * Return a pair containing the current time range values
   */
  std::pair<double, double> GetTimeRange();

private:
  const options& Options;
  window_impl& Window;
  vtkImporter* Importer = nullptr;
  interactor_impl* Interactor = nullptr;

  double TimeRange[2] = { 0.0, 0.0 };
  bool Playing = false;
  bool HasAnimation = false;
  double CurrentTime = 0;
  double DeltaTime = 0;
  bool CurrentTimeSet = false;
  int AnimationIndex = 0;
  int AvailAnimations = -1;

  vtkSmartPointer<vtkProgressBarWidget> ProgressWidget;
};
}
}
#endif
