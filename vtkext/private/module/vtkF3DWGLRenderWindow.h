/**
 * @class   vtkF3DWGLRenderWindow
 * @brief   WGL context render window
 */

#ifndef vtkF3DWGLRenderWindow_h
#define vtkF3DWGLRenderWindow_h

#include "vtkWin32OpenGLRenderWindow.h"

#include <vtkCommand.h>

class vtkF3DWGLRenderWindow : public vtkWin32OpenGLRenderWindow
{
public:
  static vtkF3DWGLRenderWindow* New();
  vtkTypeMacro(vtkF3DWGLRenderWindow, vtkWin32OpenGLRenderWindow);

  /**
   * Override to decorate with dark theme if needed.
   */
  void WindowInitialize() override;
  /**
   * Custom events invoked by this class
   */
  enum vtkCustomEvents
  {
    SystemScaleChangeEvent = vtkCommand::UserEvent + 300
  };

protected:
  vtkF3DWGLRenderWindow();
  ~vtkF3DWGLRenderWindow() override;
  /**
   * Override to handle dpi change message
   */
  LRESULT MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
  vtkF3DWGLRenderWindow(const vtkF3DWGLRenderWindow&) = delete;
  void operator=(const vtkF3DWGLRenderWindow&) = delete;
};
#endif // vtkF3DWGLRenderWindow_h
