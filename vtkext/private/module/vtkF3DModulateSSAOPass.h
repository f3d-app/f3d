/**
 * @class   vtkF3DModulateSSAOPass
 * @brief   Render pass modulating the color of props by the baked ambient occlusion.
 *
 * vtkF3DModulateSSAOPass modifies the fragment shaders of the props rendered by its delegate pass
 * to multiply the final fragment color by the ambient occlusion texture.
 *
 * @sa
 * vtkF3DHookShaderRenderPass vtkF3DBakeSSAOPass
 */

#ifndef vtkF3DModulateSSAOPass_h
#define vtkF3DModulateSSAOPass_h

#include "vtkF3DHookShaderRenderPass.h"

class vtkTextureObject;

class vtkF3DModulateSSAOPass : public vtkF3DHookShaderRenderPass
{
public:
  static vtkF3DModulateSSAOPass* New();
  vtkTypeMacro(vtkF3DModulateSSAOPass, vtkF3DHookShaderRenderPass);

  /**
   * Make the ambient occlusion texture available to the shaders, then render the delegate pass.
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Set the ambient occlusion texture.
   */
  void SetSSAOTexture(vtkTextureObject* texture)
  {
    this->SSAOTexture = texture;
  }

  /**
   * Multiply the final color by the ambient occlusion.
   */
  bool PostReplaceShaderValues(std::string& vertexShader, std::string& geometryShader,
    std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop) override;

  /**
   * Bind the ambient occlusion texture.
   */
  bool SetShaderParameters(vtkShaderProgram* program, vtkAbstractMapper* mapper, vtkProp* prop,
    vtkOpenGLVertexArrayObject* VAO = nullptr) override;

  vtkF3DModulateSSAOPass(const vtkF3DModulateSSAOPass&) = delete;
  void operator=(const vtkF3DModulateSSAOPass&) = delete;

protected:
  vtkF3DModulateSSAOPass() = default;
  ~vtkF3DModulateSSAOPass() override = default;

  vtkTextureObject* SSAOTexture = nullptr;
};

#endif
