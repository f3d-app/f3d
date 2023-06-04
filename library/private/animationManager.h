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
class window;

namespace detail
{
class interactor_impl;

class animationManager
{
public:
  animationManager() = default;
  ~animationManager() = default;

  /**
   * Initialize the animation manager, required before playing the animation.
   * Provided pointers are expected to be not null except interactor.
   */
  void Initialize(
    const options* options, window* window, interactor_impl* interactor, vtkImporter* importer);

  /**
   * Start/Stop playing the animation
   */
  void ToggleAnimation();
  void StartAnimation();
  void StopAnimation();

  /**
   * Return true if the animation manager is playing the animation
   */
  bool IsPlaying() const { return Playing; }

  /**
   * Load animation at provided time value
   */
  bool LoadAtTime(double timeValue);

  animationManager(animationManager const&) = delete;
  void operator=(animationManager const&) = delete;

protected:
  /**
   * Called by an internal timer to advance one animation tick
   */
  void Tick();

  vtkImporter* Importer = nullptr;
  window* Window = nullptr;
  interactor_impl* Interactor = nullptr;
  const options* Options = nullptr;

  double FrameRate = 30;
  double TimeRange[2] = { 0.0, 0.0 };
  bool Playing = false;
  bool HasAnimation = false;
  unsigned long CallBackId = 0;
  double CurrentTime = 0;
  bool CurrentTimeSet = false;
  std::chrono::steady_clock::time_point PreviousTick;

  vtkSmartPointer<vtkProgressBarWidget> ProgressWidget;
};
}
}
#endif
