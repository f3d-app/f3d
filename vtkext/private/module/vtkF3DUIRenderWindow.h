/**
 * @class   vtkF3DUIRenderWindow
 * @brief   A F3D render window with imgui support
 *
 */

#ifndef vtkF3DUIRenderWindow_h
#define vtkF3DUIRenderWindow_h

// TODO: how to handle cross-platform here?
#include <vtkXOpenGLRenderWindow.h>

class vtkF3DUIRenderWindow : public vtkXOpenGLRenderWindow
{
public:
  static vtkF3DUIRenderWindow* New();
  vtkTypeMacro(vtkF3DUIRenderWindow, vtkXOpenGLRenderWindow);

  vtkF3DUIRenderWindow(const vtkF3DUIRenderWindow&) = delete;
  void operator=(const vtkF3DUIRenderWindow&) = delete;

protected:
  vtkF3DUIRenderWindow() = default;
  ~vtkF3DUIRenderWindow() override;

  void OpenGLInitContext() override;

  void BlitDisplayFramebuffersToHardware() override;
};

#endif
