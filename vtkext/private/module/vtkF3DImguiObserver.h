/**
 * @class   vtkF3DImguiObserver
 * @brief   VTK ImGui event observer
 *
 * This class is used instead of the generic vtkF3DUIObserver if F3D_MODULE_UI is enabled
 */

#ifndef vtkF3DImguiObserver_h
#define vtkF3DImguiObserver_h

#include "vtkF3DUIObserver.h"

class vtkF3DRenderer;
class vtkRenderWindowInteractor;

class vtkF3DImguiObserver : public vtkF3DUIObserver
{
public:
  static vtkF3DImguiObserver* New();
  vtkTypeMacro(vtkF3DImguiObserver, vtkF3DUIObserver);

  /**
   * Install all observers to the provided interactor.
   */
  void InstallObservers(vtkRenderWindowInteractor* interactor) override;

protected:
  vtkF3DImguiObserver() = default;
  ~vtkF3DImguiObserver() override = default;

private:
  //@{
  /**
   * Callbacks called when an event is invoked by the interactor
   */
  bool MouseMove(vtkObject* caller, unsigned long, void*);
  bool MouseLeftPress(vtkObject* caller, unsigned long, void*);
  bool MouseLeftRelease(vtkObject* caller, unsigned long, void*);
  bool MouseRightPress(vtkObject* caller, unsigned long, void*);
  bool MouseRightRelease(vtkObject* caller, unsigned long, void*);
  bool MouseWheelForward(vtkObject* caller, unsigned long, void*);
  bool MouseWheelBackward(vtkObject* caller, unsigned long, void*);
  bool KeyPress(vtkObject* caller, unsigned long, void*);
  bool KeyRelease(vtkObject* caller, unsigned long, void*);
  bool Char(vtkObject* caller, unsigned long, void*);
  //@}

  /**
   * Trigger a window rendering, but render only the UI/overlay actors.
   */
  void RenderUI(vtkRenderWindowInteractor* interactor);

  vtkF3DImguiObserver(const vtkF3DImguiObserver&) = delete;
  void operator=(const vtkF3DImguiObserver&) = delete;
};

#endif
