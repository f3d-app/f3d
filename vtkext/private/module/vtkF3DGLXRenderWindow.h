/**
 * @class   vtkF3DGLXRenderWindow
 * @brief   GLX context render window with UI support
 */

#ifndef vtkF3DGLXRenderWindow_h
#define vtkF3DGLXRenderWindow_h

#include <vtkXOpenGLRenderWindow.h>

class vtkF3DGLXRenderWindow : public vtkXOpenGLRenderWindow
{
public:
  static vtkF3DGLXRenderWindow* New();
  vtkTypeMacro(vtkF3DGLXRenderWindow, vtkXOpenGLRenderWindow);

protected:
  vtkF3DGLXRenderWindow();
  ~vtkF3DGLXRenderWindow() override;

private:
  vtkF3DGLXRenderWindow(const vtkF3DGLXRenderWindow&) = delete;
  void operator=(const vtkF3DGLXRenderWindow&) = delete;
};
#endif // vtkF3DGLXRenderWindow_h
