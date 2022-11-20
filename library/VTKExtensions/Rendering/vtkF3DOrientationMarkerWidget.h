/**
 * @class   vtkF3DOrientationMarkerWidget
 * @brief   A vtkF3DOrientationMarkerWidget which is non interactive by default
 *
 * In VTK, calling vtkF3DOrientationMarkerWidget::InteractiveOff triggers a render
 * which we do not want as the renderer may not be ready to render when setting up the
 * widget. This initialize Interactive to false, removing the need to call the setter.
 * See https://gitlab.kitware.com/vtk/vtk/-/issues/18660.
 */

#ifndef vtkF3DOrientationMarkerWidget_h
#define vtkF3DOrientationMarkerWidget_h

#include "vtkOrientationMarkerWidget.h"

class vtkF3DOrientationMarkerWidget : public vtkOrientationMarkerWidget
{
public:
  static vtkF3DOrientationMarkerWidget* New();
  vtkTypeMacro(vtkF3DOrientationMarkerWidget, vtkOrientationMarkerWidget);

protected:
  vtkF3DOrientationMarkerWidget();
  ~vtkF3DOrientationMarkerWidget() override = default;

private:
  vtkF3DOrientationMarkerWidget(const vtkF3DOrientationMarkerWidget&) = delete;
  void operator=(const vtkF3DOrientationMarkerWidget&) = delete;
};

#endif
