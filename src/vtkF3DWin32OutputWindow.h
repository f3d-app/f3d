/**
 * @class   vtkF3DWin32OutputWindow
 * @brief   Custom WIN32 output window used to custom the window title
 *
 */
#ifndef vtkF3DWin32OutputWindow_h
#define vtkF3DWin32OutputWindow_h

#include "vtkWin32OutputWindow.h"

#include "vtkVersion.h"

class vtkF3DWin32OutputWindow : public vtkWin32OutputWindow
{
public:
  vtkTypeMacro(vtkF3DWin32OutputWindow, vtkWin32OutputWindow);
  static vtkF3DWin32OutputWindow* New();

  // overriden to simplify and support Unicode
  void DisplayText(const char*) override;

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201207)
  const char* GetWindowTitle() override;
#endif

protected:
  vtkF3DWin32OutputWindow() = default;
  ~vtkF3DWin32OutputWindow() override = default;

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
