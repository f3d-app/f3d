/**
 * @class   vtkF3DInteractorEventRecorder
 * @brief   A F3D dedicated version of the vtkInteractorEventRecorder
 * @sa vtkInteractorEventRecorder
 */

#ifndef vtkF3DInteractorEventRecorder_h
#define vtkF3DInteractorEventRecorder_h

#include <vtkInteractorEventRecorder.h>

class vtkF3DInteractorEventRecorder : public vtkInteractorEventRecorder
{
public:
  static vtkF3DInteractorEventRecorder* New();
  vtkTypeMacro(vtkF3DInteractorEventRecorder, vtkInteractorEventRecorder);

  /*
   * Just set the interactor without adding other dedicated observers.
   */
  void SetInteractor(vtkRenderWindowInteractor* iren) override;

  vtkF3DInteractorEventRecorder(const vtkF3DInteractorEventRecorder&) = delete;
  void operator=(const vtkF3DInteractorEventRecorder&) = delete;

protected:
  vtkF3DInteractorEventRecorder();
  ~vtkF3DInteractorEventRecorder() override = default;

  static void ProcessEvents(
    vtkObject* object, unsigned long event, void* clientdata, void* calldata);
};

#endif /* vtkF3DInteractorEventRecorder_h */
