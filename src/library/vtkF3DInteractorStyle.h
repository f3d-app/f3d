/**
 * @class   vtkF3DInteractorStyle
 * @brief   custom interactor style based on default trackball camera
 */

#ifndef vtkF3DInteractorStyle_h
#define vtkF3DInteractorStyle_h

#include <vtkCommand.h>
#include <vtkInteractorStyleTrackballCamera.h>

class F3DAnimationManager;
namespace f3d
{
class options;
}

class vtkF3DInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static vtkF3DInteractorStyle* New();
  vtkTypeMacro(vtkF3DInteractorStyle, vtkInteractorStyleTrackballCamera);

  /**
   * Custom events invoked by this class
   */
  enum vtkCustomEvents
  {
    DropFilesEvent = vtkCommand::UserEvent + 100,
    KeyPressEvent
  };

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

  vtkSetMacro(CameraMovementDisabled, bool);
  vtkGetMacro(CameraMovementDisabled, bool);

protected:
  /**
   * Overriden to disable during animation
   */
  void Dolly(double factor) override;

  bool CameraMovementDisabled = false;
};

#endif
