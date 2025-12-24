/**
 * @class   vtkF3DExternalRenderWindow
 * @brief   external context render window
 */

#ifndef vtkF3DExternalRenderWindow_h
#define vtkF3DExternalRenderWindow_h

#include "vtkGenericOpenGLRenderWindow.h"

class vtkF3DExternalRenderWindow : public vtkGenericOpenGLRenderWindow
{
public:
  static vtkF3DExternalRenderWindow* New();
  vtkTypeMacro(vtkF3DExternalRenderWindow, vtkGenericOpenGLRenderWindow);

  /**
   * Begin the rendering process using the existing context.
   */
  void Start() override;

  /**
   * An external context is always considered current.
   */
  bool IsCurrent() override
  {
    return true;
  }

  /**
   * Trigger rendering. Reimplemented to ensure OpenGL is initialized.
   */
  void Render() override;

protected:
  vtkF3DExternalRenderWindow();
  ~vtkF3DExternalRenderWindow() override;

private:
  vtkF3DExternalRenderWindow(const vtkF3DExternalRenderWindow&) = delete;
  void operator=(const vtkF3DExternalRenderWindow&) = delete;
};
#endif // vtkF3DExternalRenderWindow_h
