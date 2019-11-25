/**
 * @class   vtkF3DInteractor
 * @brief   custom interactor style based on default trackball camera
 */

#ifndef vtkF3DInteractor_h
#define vtkF3DInteractor_h

#include "Config.h"

#include <vtkInteractorStyleTrackballCamera.h>

class F3DViewer;

class vtkF3DInteractor : public vtkInteractorStyleTrackballCamera
{
public:
  static vtkF3DInteractor *New();
  vtkTypeMacro(vtkF3DInteractor, vtkInteractorStyleTrackballCamera);

  vtkF3DInteractor() = default;

  void OnChar() override;

  void SetViewer(F3DViewer *viewer) { this->Viewer = viewer; }

protected:
  F3DViewer* Viewer = nullptr;
};

#endif
