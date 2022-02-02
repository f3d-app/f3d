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
}
class vtkF3DRenderer;
class vtkImporter;
class vtkRenderWindow;

class F3DAnimationManager
{
public:
  F3DAnimationManager() = default;
  ~F3DAnimationManager() = default;

  /**
   * Initialize the animation manager, required before playing the animation
   */
  void Initialize(const f3d::options& options, vtkImporter* importer, f3d::window* window);

  /**
   * Start/Stop playing the animation
   */
  void ToggleAnimation();

  /**
   * Return true if the animation manager is playing the animation
   */
  bool IsPlaying() const { return Playing; }

  /**
   * Finalize different objects managed by the animation manager
   */
  void Finalize();

protected:
  void Tick();

  vtkImporter* Importer = nullptr;
  vtkRenderWindow* RenderWindow = nullptr;
  vtkF3DRenderer* Renderer = nullptr;

  std::set<double> TimeSteps;
  std::set<double>::iterator CurrentTimeStep;
  double FrameRate = 30;
  double TimeRange[2] = { 0.0, 0.0 };
  bool Playing = false;
  int TimerId = 0;
  unsigned long ObserverId = 0;

  vtkSmartPointer<vtkProgressBarWidget> ProgressWidget;

private:
  F3DAnimationManager(F3DAnimationManager const&) = delete;
  void operator=(F3DAnimationManager const&) = delete;
};

#endif
