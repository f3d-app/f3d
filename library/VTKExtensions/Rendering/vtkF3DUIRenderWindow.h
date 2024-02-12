/**
 * @class   vtkF3DUIRenderWindow
 * @brief   A F3D render window with imgui support
 *
 */

#ifndef vtkF3DUIRenderWindow_h
#define vtkF3DUIRenderWindow_h

// TODO: how to handle cross-platform here?
#include <vtkOpenGLRenderWindow.h>
#include <GLFW/glfw3.h>

class vtkF3DUIRenderWindow : public vtkOpenGLRenderWindow
{
public:
  static vtkF3DUIRenderWindow* New();
  vtkTypeMacro(vtkF3DUIRenderWindow, vtkOpenGLRenderWindow);

  void* GetGenericWindowId() override { return (void*)this->WindowId; }

protected:
  vtkF3DUIRenderWindow();
  ~vtkF3DUIRenderWindow() override;

  GLFWwindow *WindowId;

  void Initialize() override;
  void CreateAWindow() override;
  void DestroyWindow() override;

  void Render() override;
private:
  bool ReadyForRendering = false;

  vtkF3DUIRenderWindow(const vtkF3DUIRenderWindow&) = delete;
  void operator=(const vtkF3DUIRenderWindow&) = delete;
};

#endif
