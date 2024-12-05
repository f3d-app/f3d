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

  using CheatSheetTuple = std::tuple<std::string, std::string, std::string>;
  using CheatSheetGroup = std::pair<std::string, std::vector<CheatSheetTuple>>;

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
   * Set the metadata visibility
   * False by default
   */
  void SetMetaDataVisibility(bool show);

  /**
   * Set the filename string
   * Empty by default
   */
  void SetMetaData(const std::string& metadata);

  /**
   * Set the cheatsheet visibility
   * False by default
   */
  void SetCheatSheetVisibility(bool show);

  /**
   * Set the cheatsheet string
   * Empty by default
   */
  void SetCheatSheet(const std::vector<CheatSheetGroup>& cheatsheet);

  /**
   * Set the fps counter visibility
   * False by default
   */
  void SetFpsCounterVisibility(bool show);

  /**
   * Set the fps value
   * 0 by default
   */
  void SetFpsValue(int fps);

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

  /**
   * Render the metadata UI widget
   */
  virtual void RenderMetaData() {}

  /**
   * Render the cheatsheet UI widget
   */
  virtual void RenderCheatSheet() {}

  /**
   * Render the fps counter UI widget
   */
  virtual void RenderFpsCounter() {}

  bool FileNameVisible = false;
  std::string FileName = "";

  bool MetaDataVisible = false;
  std::string MetaData = "";

  bool CheatSheetVisible = false;
  std::vector<CheatSheetGroup> CheatSheet;

  bool FpsCounterVisible = false;
  int FpsValue = 0;

  std::string FontFile = "";

private:
  vtkF3DUIActor(const vtkF3DUIActor&) = delete;
  void operator=(const vtkF3DUIActor&) = delete;
};

#endif
