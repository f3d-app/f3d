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

  //@{
  /**
   * Use instancing or VTK geometry shader based pipeline.
   * Instancing works on GLES devices but doesn't support depth sorting.
   * Default is true.
   */
  vtkGetMacro(UseInstancing, bool);
  vtkSetMacro(UseInstancing, bool);
  vtkBooleanMacro(UseInstancing, bool);
  //@}

protected:
  vtkOpenGLPointGaussianMapperHelper* CreateHelper() override;

private:
  bool UseInstancing = true;
};

#endif
