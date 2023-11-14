/**
 * @class   vtkF3DPointSplatMapper
 * @brief   Custom F3D gaussian mapper
 *
 * This mapper is used to add a depth sort compute shader pass
 */
#ifndef vtkF3DPointSplatMapper_h
#define vtkF3DPointSplatMapper_h

#include <vtkOpenGLPointGaussianMapper.h>
#include <vtkVersion.h>

class vtkF3DPointSplatMapper : public vtkOpenGLPointGaussianMapper
{
public:
  static vtkF3DPointSplatMapper* New();
  vtkTypeMacro(vtkF3DPointSplatMapper, vtkOpenGLPointGaussianMapper);

protected:
  vtkOpenGLPointGaussianMapperHelper* CreateHelper() override;
};

#endif
