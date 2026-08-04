/**
 * @class   vtkF3DPolyDataMapper
 * @brief   Custom surface mapper used to include F3D features
 *
 * This mapper is used to add many F3D custom features:
 * - skinning and morphing capabilities
 * - support for MatCap rendering
 * - support for TAA jittering
 */

#ifndef vtkF3DPolyDataMapper_h
#define vtkF3DPolyDataMapper_h

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkVersion.h>

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
  void ReplaceShaderTCoord(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  ///@}

protected:
  vtkF3DPolyDataMapper();
  ~vtkF3DPolyDataMapper() override = default;

private:
  /**
   * Returns true if a MatCap texture is defined by the user and the actor has normals
   */
  bool RenderWithMatCap(vtkActor* actor);

  vtkNew<vtkOpenGLBufferObject> JointMatrices;
};

#endif
