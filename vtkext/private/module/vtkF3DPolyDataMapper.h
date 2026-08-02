/**
 * @class   vtkF3DPolyDataMapper
 * @brief   Custom surface mapper used to include F3D features
 *
 * This mapper is used to add support for SSBO skinning and
 * backward compatibility with old VTK versions for unlit materials.
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
   * Modify the shaders to handle color space
   */
  void ReplaceShaderColor(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  void ReplaceShaderLight(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;
  ///@}

  /**
   * Set the SSBO for skinning if needed
   */
  void SetCustomUniforms(vtkOpenGLHelper& cellBO, vtkActor* actor) override;

protected:
  vtkF3DPolyDataMapper() = default;
  ~vtkF3DPolyDataMapper() override = default;

private:
  vtkNew<vtkOpenGLBufferObject> JointMatrices;
  bool HasSSBOSkinning = false;
};

#endif
