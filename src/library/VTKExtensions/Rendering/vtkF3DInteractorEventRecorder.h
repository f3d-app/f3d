/**
 * @class   vtkF3DInteractorEventRecorder
 * @brief   A F3D dedicated version of the vtkInteractorEventRecorder
 * @sa vtkInteractorEventRecorder
 */

#ifndef vtkF3DInteractorEventRecorder_h
#define vtkF3DInteractorEventRecorder_h

#include <vtkInteractorEventRecorder.h>

#include <vtkVersion.h>

class vtkF3DInteractorEventRecorder : public vtkInteractorEventRecorder
{
public:
  static vtkF3DInteractorEventRecorder* New();
  vtkTypeMacro(vtkF3DInteractorEventRecorder, vtkInteractorEventRecorder);

  /*
   * Just set the interactor without adding other dedicated observers.
   */
  void SetInteractor(vtkRenderWindowInteractor* iren) override;

protected:
  vtkF3DInteractorEventRecorder();
  ~vtkF3DInteractorEventRecorder() override = default;

  static void ProcessEvents(
    vtkObject* object, unsigned long event, void* clientdata, void* calldata);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 1, 20220510)
  /**
   * A method that write an event to log file, with a trailing 0
   */
  void WriteEvent(const char* event, int pos[2], int modifiers, int keyCode, int repeatCount,
    char* keySym) override;

  /**
   * A method that parse a event line and invoke the corresponding event
   * support reading a serialized vtkStringArray
   */
  void ReadEvent(const std::string& line) override;
#endif

private:
  vtkF3DInteractorEventRecorder(const vtkF3DInteractorEventRecorder&) = delete;
  void operator=(const vtkF3DInteractorEventRecorder&) = delete;
};

#endif /* vtkF3DInteractorEventRecorder_h */
