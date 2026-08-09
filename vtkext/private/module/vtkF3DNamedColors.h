/**
 * @class   vtkF3DNamedColors
 * @brief   Registers F3DStyle colors
 *
 * This class registers F3DStyle colors on being constructed
 */

#ifndef vtkF3DNamedColors_h
#define vtkF3DNamedColors_h

#include "vtkNamedColors.h"

class vtkF3DNamedColors : public vtkNamedColors
{
public:
  static vtkF3DNamedColors* New();
  vtkTypeMacro(vtkF3DNamedColors, vtkNamedColors);

protected:
  vtkF3DNamedColors();
  ~vtkF3DNamedColors() override = default;
};

#endif
