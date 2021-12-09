/**
 * @class   vtkF3DInteractorStyle3D
 * @brief   custom interactor style based on default trackball camera
 */

#ifndef vtkF3DInteractorStyle3D_h
#define vtkF3DInteractorStyle3D_h

#include <vtkInteractorStyleTrackballCamera.h>

class vtkF3DInteractionHandler;
class F3DAnimationManager;
struct F3DOptions;

class vtkF3DInteractorStyle3D : public vtkInteractorStyleTrackballCamera
{
public:
  static vtkF3DInteractorStyle3D* New();
  vtkTypeMacro(vtkF3DInteractorStyle3D, vtkInteractorStyleTrackballCamera);

  /**
   * Handle key presses
   */
  void OnKeyPress() override;

  /**
   * Disable base class features
   */
  void OnChar() override {}

  /**
   * Disable base class features
   */
  void OnTimer() override {}

  /**
   * Handle drop files
   */
  void OnDropFiles(vtkStringArray* files) override;

  /**
   * Overriden for turntable mode
   */
  void Rotate() override;

  //@{
  /**
   * Overriden to disable during animation
   */
  void Spin() override;
  void Pan() override;
  void Dolly() override;
  //@}

  /**
   * Overriden to rotate the skybox as well
   */
  void EnvironmentRotate() override;

  /**
   * Set animation manager
   */
  void SetAnimationManager(const F3DAnimationManager& mgr) { this->AnimationManager = &mgr; }

  /**
   * Set interaction handler
   */
  void SetInteractionHandler(vtkF3DInteractionHandler* handler) { this->InteractionHandler = handler; }

  /**
   * Set options
   */
  void SetOptions(const F3DOptions& options) { this->Options = &options; }

protected:
  /**
   * Overriden to disable during animation
   */
  void Dolly(double factor) override;

  virtual bool IsUserInteractionBlocked();

  const F3DAnimationManager* AnimationManager = nullptr;
  const F3DOptions* Options = nullptr;
  vtkF3DInteractionHandler* InteractionHandler = nullptr;
};

#endif
