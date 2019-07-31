#include <vtkInteractorStyleTrackballActor.h>

class vtkF3DInteractor : public vtkInteractorStyleTrackballActor
{
public:
  static vtkF3DInteractor *New();
  vtkTypeMacro(vtkF3DInteractor, vtkInteractorStyleTrackballActor);

  vtkF3DInteractor() = default;

  void OnChar() override;

  void OnKeyPress() override;
};
