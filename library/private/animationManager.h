/**
 * @class   animationManager
 * @brief   A private class managing animation
 */

#ifndef f3d_animationManager_h
#define f3d_animationManager_h

#include <vtkNew.h>
#include <vtkProgressBarWidget.h>
#include <vtkSmartPointer.h>

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
   * Provided pointers are expected to be not null.
   */
  void Initialize(
    const options* options, interactor_impl* interactor, window* window, vtkImporter* importer);

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

protected:
  /**
   * Called by an internal timer to advance one animation tick
   */
  void Tick();

  vtkImporter* Importer = nullptr;
  window* Window;
  interactor_impl* Interactor;
  const options* Options;

  std::set<double> TimeSteps;
  std::set<double>::iterator CurrentTimeStep;
  double FrameRate = 30;
  double TimeRange[2] = { 0.0, 0.0 };
  bool Playing = false;
  bool HasAnimation = false;
  unsigned long CallBackId = 0;

  vtkSmartPointer<vtkProgressBarWidget> ProgressWidget;

private:
  animationManager(animationManager const&) = delete;
  void operator=(animationManager const&) = delete;
};
}
}
#endif
