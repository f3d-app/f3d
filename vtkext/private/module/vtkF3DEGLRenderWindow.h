/**
 * @class   vtkF3DEGLRenderWindow
 * @brief   EGL context render window
 */

#ifndef vtkF3DEGLRenderWindow_h
#define vtkF3DEGLRenderWindow_h

#include <vtkXOpenGLRenderWindow.h>

class vtkF3DEGLRenderWindow : public vtkXOpenGLRenderWindow
{
public:
  static vtkF3DEGLRenderWindow* New();
  vtkTypeMacro(vtkF3DEGLRenderWindow, vtkXOpenGLRenderWindow);

protected:
  vtkF3DEGLRenderWindow();
  ~vtkF3DEGLRenderWindow() override;

private:
  vtkF3DEGLRenderWindow(const vtkF3DEGLRenderWindow&) = delete;
  void operator=(const vtkF3DEGLRenderWindow&) = delete;
};
#endif // vtkF3DEGLRenderWindow_h
