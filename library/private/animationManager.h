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
   * Return true if at least one animation is available, false otherwise.
   */
  bool Initialize();

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
   * Load animation at provided time value
   */
  bool LoadAtTime(double timeValue);

  animationManager(animationManager const&) = delete;
  void operator=(animationManager const&) = delete;

  /**
   * Set a time range pointer to the current time range values
   */
  void GetTimeRange(double timeRange[2]);

private:
  /**
   * Called by an internal timer to advance one animation tick
   */
  void Tick();

  const options& Options;
  window_impl& Window;
  vtkImporter* Importer = nullptr;
  interactor_impl* Interactor = nullptr;

  double TimeRange[2] = { 0.0, 0.0 };
  bool Playing = false;
  bool HasAnimation = false;
  unsigned long CallBackId = 0;
  double CurrentTime = 0;
  bool CurrentTimeSet = false;
  int AnimationIndex = 0;
  int AvailAnimations = -1;
  std::chrono::steady_clock::time_point PreviousTick;

  vtkSmartPointer<vtkProgressBarWidget> ProgressWidget;
};
}
}
#endif
