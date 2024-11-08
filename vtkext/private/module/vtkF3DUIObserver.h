/**
 * @class   vtkF3DUIObserver
 * @brief   An interface for UI event observer
 *
 * Implemented by vtkF3DImguiObserver
 */

#ifndef vtkF3DUIObserver_h
#define vtkF3DUIObserver_h

#include <vtkObject.h>

class vtkRenderWindowInteractor;

class vtkF3DUIObserver : public vtkObject
{
public:
  static vtkF3DUIObserver* New();
  vtkTypeMacro(vtkF3DUIObserver, vtkObject);

  /**
   * Install all observers to the provided interactor.
   */
  virtual void InstallObservers(vtkRenderWindowInteractor*) {}

  vtkF3DUIObserver(const vtkF3DUIObserver&) = delete;
  void operator=(const vtkF3DUIObserver&) = delete;

protected:
  vtkF3DUIObserver() = default;
  ~vtkF3DUIObserver() override = default;
};

#endif
