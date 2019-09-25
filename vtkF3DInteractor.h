#include <vtkInteractorStyleTrackballCamera.h>

class vtkF3DInteractor : public vtkInteractorStyleTrackballCamera
{
public:
  static vtkF3DInteractor *New();
  vtkTypeMacro(vtkF3DInteractor, vtkInteractorStyleTrackballCamera);

  vtkF3DInteractor() = default;

  void OnChar() override;

  void OnKeyPress() override;
};
