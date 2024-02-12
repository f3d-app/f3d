/**
 * @class   vtkF3DUIRenderWindow
 * @brief   A F3D render window with imgui support
 *
 */

#ifndef vtkF3DUIWindowInteractor_h
#define vtkF3DUIWindowInteractor_h

#include <vtkRenderWindowInteractor.h>

class vtkF3DUIWindowInteractor : public vtkRenderWindowInteractor
{
public:
  static vtkF3DUIWindowInteractor* New();
  vtkTypeMacro(vtkF3DUIWindowInteractor, vtkRenderWindowInteractor);

protected:
  vtkF3DUIWindowInteractor();
  ~vtkF3DUIWindowInteractor() override;

  void Start() override;

private:
  vtkF3DUIWindowInteractor(const vtkF3DUIWindowInteractor&) = delete;
  void operator=(const vtkF3DUIWindowInteractor&) = delete;
};

#endif
