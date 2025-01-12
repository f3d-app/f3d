/**
 * @class   vtkF3DUIActor
 * @brief   A basic implementation for UI context handler and renderer
 *
 * This is overridden by vtkF3DImguiActor if F3D_MODULE_UI is enabled
 */

#ifndef vtkF3DUIActor_h
#define vtkF3DUIActor_h

#include <vtkProp.h>

#include <deque>

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
  virtual void Initialize(vtkOpenGLRenderWindow*)
  {
  }

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
   * Set the console visibility
   * False by default
   */
  void SetConsoleVisibility(bool show);

  /**
   * Set the console badge enabled status
   * False by default
   */
  void SetConsoleBadgeEnabled(bool enabled);

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
   * Updates the fps value
   * 0 by default
   */
  void UpdateFpsValue(const double elapsedFrameTime);

  /**
   * Set the font file path
   * Use Inter font by default if empty
   */
  void SetFontFile(const std::string& font);

  /**
   * Set the font scale
   */
  void SetFontScale(const double fontScale);

  /**
   * Render the UI actor
   */
  int RenderOverlay(vtkViewport* vp) override;

  /**
   * Set the delta time (time between UI frames) in seconds
   */
  virtual void SetDeltaTime(double)
  {
  }

protected:
  vtkF3DUIActor();
  ~vtkF3DUIActor() override;

  /**
   * Called at the beginning of the rendering step
   */
  virtual void StartFrame(vtkOpenGLRenderWindow*)
  {
  }

  /**
   * Called at the end of the rendering step
   */
  virtual void EndFrame(vtkOpenGLRenderWindow*)
  {
  }

  /**
   * Render the filename UI widget
   */
  virtual void RenderFileName()
  {
  }

  /**
   * Render the metadata UI widget
   */
  virtual void RenderMetaData()
  {
  }

  /**
   * Render the cheatsheet UI widget
   */
  virtual void RenderCheatSheet()
  {
  }

  /**
   * Render the fps counter UI widget
   */
  virtual void RenderFpsCounter()
  {
  }

  /**
   * Render the console widget
   */
  virtual void RenderConsole()
  {
  }

  /**
   * Render the console badge
   */
  virtual void RenderConsoleBadge()
  {
  }

  bool FileNameVisible = false;
  std::string FileName = "";

  bool MetaDataVisible = false;
  std::string MetaData = "";

  bool CheatSheetVisible = false;
  std::vector<CheatSheetGroup> CheatSheet;

  bool ConsoleVisible = false;
  bool ConsoleBadgeEnabled = false;

  bool FpsCounterVisible = false;

  // deque instead of queue to allow for iteration
  std::deque<double> FrameTimes;

  double TotalFrameTimes = 0.0;
  int FpsValue = 0;

  std::string FontFile = "";
  double FontScale = 1.0;

private:
  vtkF3DUIActor(const vtkF3DUIActor&) = delete;
  void operator=(const vtkF3DUIActor&) = delete;

  bool Initialized = false;
};

#endif
