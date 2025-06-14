/**
 * @class   vtkF3DImguiActor
 * @brief   A ImGui context handler and renderer
 *
 * This class is used instead of the generic vtkF3DUIActor if F3D_MODULE_UI is enabled
 */

#ifndef vtkF3DImguiActor_h
#define vtkF3DImguiActor_h

#include "vtkF3DUIActor.h"

#include <memory>

class vtkOpenGLRenderWindow;
class vtkWindow;

class vtkF3DImguiActor : public vtkF3DUIActor
{
public:
  static vtkF3DImguiActor* New();
  vtkTypeMacro(vtkF3DImguiActor, vtkF3DUIActor);

  /**
   * Initialize the UI actor resources
   */
  void Initialize(vtkOpenGLRenderWindow* renwin) override;

  /**
   * Release the UI actor resources
   */
  void ReleaseGraphicsResources(vtkWindow* w) override;

  /**
   * Set imgui::DeltaTime, with time in seconds
   */
  void SetDeltaTime(double time) override;

protected:
  vtkF3DImguiActor();
  ~vtkF3DImguiActor() override;

private:
  struct Internals;
  std::unique_ptr<Internals> Pimpl;

  /**
   * Called at the beginning of the rendering step
   * Initialize the imgui context if needed and setup a new frame
   */
  void StartFrame(vtkOpenGLRenderWindow* renWin) override;

  /**
   * Called at the end of the rendering step
   * Finish the imgui frame and render data on the GPU
   */
  void EndFrame(vtkOpenGLRenderWindow* renWin) override;

  /**
   * Render the dropzone UI widget
   */
  void RenderDropZone() override;

  /**
   * Render the filename UI widget
   */
  void RenderFileName() override;

  /**
   * Render the metadata UI widget
   */
  void RenderMetaData() override;

  /**
   * Render the cheatsheet UI widget
   */
  void RenderCheatSheet() override;

  /**
   * Render the fps UI widget
   */
  void RenderFpsCounter() override;

  /**
   * Render the console widget
   */
  void RenderConsole(bool) override;

  /**
   * Render the console badge
   */
  void RenderConsoleBadge() override;

private:
  vtkF3DImguiActor(const vtkF3DImguiActor&) = delete;
  void operator=(const vtkF3DImguiActor&) = delete;
};

#endif
