/**
 * @class   vtkF3DWin32OutputWindow
 * @brief   Custom WIN32 output window used to custom the window title
 *
 */
#ifndef vtkF3DWin32OutputWindow_h
#define vtkF3DWin32OutputWindow_h

#include <vtkVersion.h>
#include <vtkWin32OutputWindow.h>

class vtkF3DWin32OutputWindow : public vtkWin32OutputWindow
{
public:
  vtkTypeMacro(vtkF3DWin32OutputWindow, vtkWin32OutputWindow);
  static vtkF3DWin32OutputWindow* New();

  /**
   * Overridden to simplify and support Unicode
   */
  void DisplayText(const char*) override;

  /**
   * Simple method that display a window with an OK button
   * for the user to click on it.
   */
  void WaitForUser();

  /**
   * Get the window title to display
   */
  const char* GetWindowTitle() override;

protected:
  vtkF3DWin32OutputWindow();
  ~vtkF3DWin32OutputWindow() override = default;

  int Initialize() override;

  void* EditControlHandle = nullptr;

private:
  vtkF3DWin32OutputWindow(const vtkF3DWin32OutputWindow&) = delete;
  void operator=(const vtkF3DWin32OutputWindow&) = delete;
};

#endif
