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

// WindowTitle needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7460
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201207)
  /**
   * Get the window title to display
   */
  const char* GetWindowTitle() override;
#endif

protected:
  vtkF3DWin32OutputWindow();
  ~vtkF3DWin32OutputWindow() override = default;

// virtual Initialize needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7460
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201207)
  int Initialize() override;
#else
  int Initialize();
#endif

  void* EditControlHandle = nullptr;

private:
  vtkF3DWin32OutputWindow(const vtkF3DWin32OutputWindow&) = delete;
  void operator=(const vtkF3DWin32OutputWindow&) = delete;
};

#endif
