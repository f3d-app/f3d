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
class vtkRenderWindowInteractor;

class vtkF3DInteractionHandler : public vtkObject
{
public:
  vtkTypeMacro(vtkF3DInteractionHandler, vtkObject);

  static vtkF3DInteractionHandler* GetInstance()
  {
    if (!vtkF3DInteractionHandler::Instance)
    {
      vtkF3DInteractionHandler::Instance = vtkF3DInteractionHandler::New();
    }
    return vtkF3DInteractionHandler::Instance;
  }

  void SetupInteractorStyles(vtkRenderWindowInteractor*, F3DAnimationManager*, F3DOptions*);

  void OnDropFiles(vtkInteractorStyle*, vtkStringArray*);

  /**
   * Handle the key press events.
   * Returns true if key has been handled, false otherwise.
   */
  bool HandleKeyPress(vtkInteractorStyle*);

protected:
  vtkF3DInteractionHandler() = default;
  ~vtkF3DInteractionHandler() override = default;
  static vtkF3DInteractionHandler* New();

private:
  vtkF3DInteractionHandler(const vtkF3DInteractionHandler&) = delete;
  void operator=(const vtkF3DInteractionHandler&) = delete;

  static vtkF3DInteractionHandler* Instance;

  vtkRenderWindowInteractor* Interactor = nullptr;
  vtkNew<vtkF3DInteractorStyle2D> Style2D;
  vtkNew<vtkF3DInteractorStyle3D> Style3D;

  int WindowSize[2] = { -1, -1 };
  int WindowPos[2] = { 0, 0 };
};

#endif
