/**
 * @class   F3DAnimationManager
 * @brief   The animation manager class
 *
 */

#ifndef F3DAnimationManager_h
#define F3DAnimationManager_h

#include <vtkNew.h>
#include <vtkProgressBarWidget.h>
#include <vtkSmartPointer.h>

#include <set>

namespace f3d
{
class options;
class window;
class interactor;
}
class vtkF3DRenderer;
class vtkImporter;
class vtkRenderWindow;

class F3DAnimationManager
{
public:
  F3DAnimationManager() = default;
  ~F3DAnimationManager() = default; //TODO Stop animation ?

  /**
   * Initialize the animation manager, required before playing the animation
   */
  void Initialize(const f3d::options* options, f3d::interactor* interactor, f3d::window* window,
    vtkImporter* importer);

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
  void Tick();

  vtkImporter* Importer = nullptr;
  f3d::window* Window;
  f3d::interactor* Interactor;
  const f3d::options* Options;

  std::set<double> TimeSteps;
  std::set<double>::iterator CurrentTimeStep;
  double FrameRate = 30;
  double TimeRange[2] = { 0.0, 0.0 };
  bool Playing = false;
  bool HasAnimation = false;
  unsigned long CallBackId = 0;

  vtkSmartPointer<vtkProgressBarWidget> ProgressWidget;

private:
  F3DAnimationManager(F3DAnimationManager const&) = delete;
  void operator=(F3DAnimationManager const&) = delete;
};

#endif
