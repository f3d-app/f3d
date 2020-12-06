/**
 * @class   vtkF3DConsoleOutputWindow
 * @brief   Custom console output window
 *
 */
#ifndef vtkF3DConsoleOutputWindow_h
#define vtkF3DConsoleOutputWindow_h

#include "vtkOutputWindow.h"

class vtkF3DConsoleOutputWindow : public vtkOutputWindow
{
public:
  vtkTypeMacro(vtkF3DConsoleOutputWindow, vtkOutputWindow);
  static vtkF3DConsoleOutputWindow* New();

  void DisplayText(const char*) override;

protected:
  vtkF3DConsoleOutputWindow();
  ~vtkF3DConsoleOutputWindow() override = default;

private:
  vtkF3DConsoleOutputWindow(const vtkF3DConsoleOutputWindow&) = delete;
  void operator=(const vtkF3DConsoleOutputWindow&) = delete;
};

#endif
