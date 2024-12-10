/**
 * @class   vtkF3DConsoleOutputWindow
 * @brief   Custom console output window
 *
 */
#ifndef vtkF3DConsoleOutputWindow_h
#define vtkF3DConsoleOutputWindow_h

#include <vtkOutputWindow.h>

#include <vtkCommand.h>

class vtkF3DConsoleOutputWindow : public vtkOutputWindow
{
public:
  vtkTypeMacro(vtkF3DConsoleOutputWindow, vtkOutputWindow);
  static vtkF3DConsoleOutputWindow* New();

  /**
   * Custom events invoked by this class
   */
  enum vtkCustomEvents
  {
    TriggerEvent = vtkCommand::UserEvent + 200,
    ShowEvent,
    HideEvent
  };

  /**
   * Reimplemented to support coloring
   */
  void DisplayText(const char*) override;

  //@{
  /**
   * Set/Get the coloring usage.
   * Default is true.
   */
  vtkSetMacro(UseColoring, bool);
  vtkGetMacro(UseColoring, bool);
  //@}

  vtkF3DConsoleOutputWindow(const vtkF3DConsoleOutputWindow&) = delete;
  void operator=(const vtkF3DConsoleOutputWindow&) = delete;

protected:
  vtkF3DConsoleOutputWindow();
  ~vtkF3DConsoleOutputWindow() override = default;

private:
  bool UseColoring = true;
};

#endif
