/**
 * @class   vtkF3DPolyDataMapper
 * @brief   Custom surface mapper used to include skinning and morphing for glTF format
 *
 */

#ifndef vtkF3DPolyDataMapper_h
#define vtkF3DPolyDataMapper_h

#include <vtkOpenGLPolyDataMapper.h>

class vtkF3DPolyDataMapper : public vtkOpenGLPolyDataMapper
{
public:
  static vtkF3DPolyDataMapper* New();
  vtkTypeMacro(vtkF3DPolyDataMapper, vtkOpenGLPolyDataMapper);

  /**
   * Modify the shaders to include skinning and morphing capabilities
   */
  void ReplaceShaderValues(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;

  ///@{
  /**
   * Modify the shaders to use MatCap if enabled
   */
  void ReplaceShaderColor(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  void ReplaceShaderLight(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  ///@}

protected:
  vtkF3DPolyDataMapper();
  ~vtkF3DPolyDataMapper() override = default;

private:
  bool RenderWithMatCap(vtkActor* actor);
};

#endif
