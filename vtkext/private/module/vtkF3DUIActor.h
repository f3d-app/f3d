/**
 * @class   vtkF3DUIActor
 * @brief   A basic implementation for UI context handler and renderer
 *
 * This is overridden by vtkF3DImguiActor if F3D_MODULE_UI is enabled
 */

#ifndef vtkF3DUIActor_h
#define vtkF3DUIActor_h

#include <vtkProp.h>

class vtkOpenGLRenderWindow;

class vtkF3DUIActor : public vtkProp
{
public:
  static vtkF3DUIActor* New();
  vtkTypeMacro(vtkF3DUIActor, vtkProp);

  /**
  * Initialize the UI actor resources
  */
  virtual void Initialize(vtkOpenGLRenderWindow*) {}

  /**
   * Set the filename visibility
   * False by default
   */
  void SetFileNameVisibility(bool show);

  /**
   * Set the filename string
   * Empty by default
   */
  void SetFileName(const std::string& filename);

  /**
   * Set the font file path
   * Use Inter font by default if empty
   */
  void SetFontFile(const std::string& font);

  /**
   * Render the UI actor
   */
  int RenderOverlay(vtkViewport* vp) override;

protected:
  vtkF3DUIActor();
  ~vtkF3DUIActor() override;

  /**
   * Called at the beginning of the rendering step
   */
  virtual void StartFrame(vtkOpenGLRenderWindow*) {}

  /**
   * Called at the end of the rendering step
   */
  virtual void EndFrame(vtkOpenGLRenderWindow*) {}

  /**
   * Render the filename UI widget
   */
  virtual void RenderFileName() {}

  bool FileNameVisible = false;
  std::string FileName = "";
  std::string FontFile = "";

private:
  vtkF3DUIActor(const vtkF3DUIActor&) = delete;
  void operator=(const vtkF3DUIActor&) = delete;
};

#endif
