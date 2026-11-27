/**
 * @class   vtkF3DFixColorSpacePass
 * @brief   Render pass removing the gamma correction applied by the shaders of the actors.
 *
 * vtkF3DFixColorSpacePass modifies the fragment shaders of the actors to remove the final
 * gamma correction in case linear color space is expected.
 *
 * @sa
 * vtkF3DHookShaderRenderPass
 */

#ifndef vtkF3DFixColorSpacePass_h
#define vtkF3DFixColorSpacePass_h

#include "vtkF3DHookShaderRenderPass.h"

class vtkF3DFixColorSpacePass : public vtkF3DHookShaderRenderPass
{
public:
  static vtkF3DFixColorSpacePass* New();
  vtkTypeMacro(vtkF3DFixColorSpacePass, vtkF3DHookShaderRenderPass);

  /**
   * Remove the final gamma correction from the fragment shader.
   */
  bool PostReplaceShaderValues(std::string& vertexShader, std::string& geometryShader,
    std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop) override;

  vtkF3DFixColorSpacePass(const vtkF3DFixColorSpacePass&) = delete;
  void operator=(const vtkF3DFixColorSpacePass&) = delete;

protected:
  vtkF3DFixColorSpacePass() = default;
  ~vtkF3DFixColorSpacePass() override = default;
};

#endif
