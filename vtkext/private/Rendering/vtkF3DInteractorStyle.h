/**
 * @class   vtkF3DInteractorStyle
 * @brief   custom interactor style based on default trackball camera
 */

#ifndef vtkF3DInteractorStyle_h
#define vtkF3DInteractorStyle_h

#include <vtkCommand.h>
#include <vtkInteractorStyleTrackballCamera.h>

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
  void OnChar() override
  {
  }

  /**
   * Disable base class features
   */
  void OnTimer() override
  {
  }

  /**
   * Handle drop files
   */
  void OnDropFiles(vtkStringArray* files) override;

  /**
   * Overridden for turntable mode
   */
  void Rotate() override;

  ///@{
  /**
   * Overridden to support being disabled
   */
  void Spin() override;
  void Pan() override;
  void Dolly() override;
  ///@}

  /**
   * Dolly the renderer's camera to a specific point
   */
  static void DollyToPosition(double fact, int* position, vtkRenderer* renderer);

  /**
   * Overridden to rotate the skybox as well
   */
  void EnvironmentRotate() override;

  ///@{
  /**
   * Set/Get is camera movement are disabled
   */
  vtkSetMacro(CameraMovementDisabled, bool);
  vtkGetMacro(CameraMovementDisabled, bool);
  ///@}

  /**
   * Update the renderer as needed, especially
   * the camera clipping range
   */
  void UpdateRendererAfterInteraction();

  /**
   * Reimplemented to always return the first
   * renderer as this is the only one used
   * for interaction. This is needed for performance
   * reasons.
   */
  void FindPokedRenderer(int vtkNotUsed(x), int vtkNotUsed(y));

protected:
  /**
   * Overridden to support being disabled
   */
  void Dolly(double factor) override;

  bool CameraMovementDisabled = false;
};

#endif
