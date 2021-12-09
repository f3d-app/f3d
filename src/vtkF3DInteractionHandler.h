/**
 * @class   vtkF3DInteractionHandler
 * @brief   The key handler utility class
 *
 */

#ifndef vtkF3DInteractionHandler_h
#define vtkF3DInteractionHandler_h

#include <vtkNew.h>
#include <vtkObject.h>

#include "vtkF3DInteractorStyle2D.h"
#include "vtkF3DInteractorStyle3D.h"

class F3DAnimationManager;
class vtkInteractorStyle;
class vtkStringArray;
class vtkRenderer;
class vtkRenderWindowInteractor;

class vtkF3DInteractionHandler : public vtkObject
{
public:
  static vtkF3DInteractionHandler* New();
  vtkTypeMacro(vtkF3DInteractionHandler, vtkObject);

  void SetupInteractorStyles(vtkRenderWindowInteractor*, F3DAnimationManager*, F3DOptions*);

  void SetDefaultStyle(vtkRenderer*);

  void OnDropFiles(vtkInteractorStyle*, vtkStringArray*);

  /**
   * Handle the key press events.
   * Returns true if key has been handled, false otherwise.
   */
  bool HandleKeyPress(vtkInteractorStyle*);

protected:
  vtkF3DInteractionHandler() = default;
  ~vtkF3DInteractionHandler() override = default;

private:
  vtkF3DInteractionHandler(const vtkF3DInteractionHandler&) = delete;
  void operator=(const vtkF3DInteractionHandler&) = delete;

  vtkRenderWindowInteractor* Interactor = nullptr;
  vtkNew<vtkF3DInteractorStyle2D> Style2D;
  vtkNew<vtkF3DInteractorStyle3D> Style3D;

  int WindowSize[2] = { -1, -1 };
  int WindowPos[2] = { 0, 0 };
};

#endif
