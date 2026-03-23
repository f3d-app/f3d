/**
 * @class   vtkF3DNamedColors
 * @brief
 */

#ifndef vtkF3DNamedColors_h
#define vtkF3DNamedColors_h

#include <string>
#include <vtkNamedColors.h>

class vtkF3DNamedColors : public vtkNamedColors
{
public:
  static vtkF3DNamedColors* New();
  vtkTypeMacro(vtkF3DNamedColors, vtkNamedColors);

  /**
   * Check if color exists by name
   */
  bool ColorExists(const std::string& color);

  /**
   * Get color by name
   */
  void GetColor(const std::string& color, double rgba[4]);

protected:
  vtkF3DNamedColors();
  ~vtkF3DNamedColors() override;
};

#endif