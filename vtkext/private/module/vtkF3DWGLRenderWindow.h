/**
 * @class   vtkF3DWGLRenderWindow
 * @brief   WGL context render window
 */

#ifndef vtkF3DWGLRenderWindow_h
#define vtkF3DWGLRenderWindow_h

#include "vtkWin32OpenGLRenderWindow.h"

class vtkF3DWGLRenderWindow : public vtkWin32OpenGLRenderWindow
{
public:
  static vtkF3DWGLRenderWindow* New();
  vtkTypeMacro(vtkF3DWGLRenderWindow, vtkWin32OpenGLRenderWindow);

  /**
   * Override to decorate with dark theme if needed.
   */
  void WindowInitialize() override;

protected:
  vtkF3DWGLRenderWindow();
  ~vtkF3DWGLRenderWindow() override;

private:
  vtkF3DWGLRenderWindow(const vtkF3DWGLRenderWindow&) = delete;
  void operator=(const vtkF3DWGLRenderWindow&) = delete;
};
#endif // vtkF3DWGLRenderWindow_h
