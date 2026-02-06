/**
 * @class   vtkF3D2DInteractorStyle
 * @brief   Custom interactor style for 2D viewing (pan/zoom without rotation)
 */

#ifndef vtkF3D2DInteractorStyle_h
#define vtkF3D2DInteractorStyle_h

#include <vtkCommand.h>
#include <vtkInteractorStyleImage.h>

class vtkF3D2DInteractorStyle : public vtkInteractorStyleImage
{
public:
  static vtkF3D2DInteractorStyle* New();
  vtkTypeMacro(vtkF3D2DInteractorStyle, vtkInteractorStyleImage);

  enum vtkCustomEvents
  {
    DropFilesEvent = vtkCommand::UserEvent + 100,
    KeyPressEvent
  };

  ///@{
  /**
   * Override mouse button handlers for 2D navigation.
   */
  void OnLeftButtonDown() override;
  void OnLeftButtonUp() override;
  void OnMiddleButtonDown() override;
  void OnMiddleButtonUp() override;
  void OnRightButtonDown() override;
  void OnRightButtonUp() override;
  ///@}

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

  ///@{
  /**
   * Overridden to support being disabled
   */
  void Pan() override;
  void Dolly() override;
  ///@}

  /**
   * Dolly the renderer's camera to a specific point
   */
  static void DollyToPosition(double fact, int* position, vtkRenderer* renderer);

  ///@{
  /**
   * Set/Get if camera movement is disabled
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
   * for interaction.
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
