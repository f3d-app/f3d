/**
 * @class   vtkF3DUIObserver
 * @brief   A basic implementation for UI event observer
 *
 * This is overridden by vtkF3DImguiObserver if F3D_MODULE_UI is enabled
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
  virtual void InstallObservers(vtkRenderWindowInteractor*)
  {
  }

protected:
  vtkF3DUIObserver() = default;
  ~vtkF3DUIObserver() override = default;

private:
  vtkF3DUIObserver(const vtkF3DUIObserver&) = delete;
  void operator=(const vtkF3DUIObserver&) = delete;
};

#endif
