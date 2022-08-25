/**
 * @class   vtkF3DNoRenderWindow
 * @brief   A vtkRenderWindow that do no render anything
 * @sa vtkRenderWindow
 */

#ifndef vtkF3DNoRenderWindow_h
#define vtkF3DNoRenderWindow_h

#include <vtkRenderWindow.h>

class vtkF3DNoRenderWindow : public vtkRenderWindow
{
public:
  static vtkF3DNoRenderWindow* New();
  vtkTypeMacro(vtkF3DNoRenderWindow, vtkRenderWindow);

protected:
  vtkF3DNoRenderWindow() = default;
  ~vtkF3DNoRenderWindow() override = default;

private:
  vtkF3DNoRenderWindow(const vtkF3DNoRenderWindow&) = delete;
  void operator=(const vtkF3DNoRenderWindow&) = delete;
};

#endif /* vtkF3DNoRenderWindow_h */
