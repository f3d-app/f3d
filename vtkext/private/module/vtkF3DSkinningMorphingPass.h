/**
 * @class   vtkF3DSkinningMorphingPass
 * @brief   Render pass handling the skinning and morphing of actors.
 *
 * vtkF3DSkinningMorphingPass maps the skinning and morphing arrays to vertex attributes
 * and modifies the vertex shaders of the actors to apply the skinning and morphing.
 *
 * The delegate pass is expected to render the props.
 *
 * @sa
 * vtkF3DHookShaderRenderPass
 */

#ifndef vtkF3DSkinningMorphingPass_h
#define vtkF3DSkinningMorphingPass_h

#include "vtkF3DHookShaderRenderPass.h"

class vtkActor;
class vtkPolyData;
class vtkPolyDataMapper;

class vtkF3DSkinningMorphingPass : public vtkF3DHookShaderRenderPass
{
public:
  static vtkF3DSkinningMorphingPass* New();
  vtkTypeMacro(vtkF3DSkinningMorphingPass, vtkF3DHookShaderRenderPass);

  /**
   * Map the skinning and morphing arrays of the props, then render the delegate pass.
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Modify the vertex shader to apply skinning and morphing.
   */
  bool PreReplaceShaderValues(std::string& vertexShader, std::string& geometryShader,
    std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop) override;
  bool PostReplaceShaderValues(std::string& vertexShader, std::string& geometryShader,
    std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop) override;

  vtkF3DSkinningMorphingPass(const vtkF3DSkinningMorphingPass&) = delete;
  void operator=(const vtkF3DSkinningMorphingPass&) = delete;

protected:
  vtkF3DSkinningMorphingPass() = default;
  ~vtkF3DSkinningMorphingPass() override = default;

private:
  void MapAttributes(vtkPolyDataMapper* polyMapper, vtkPolyData* input);
  void ReplaceSkinningMorphing(std::string& vertexShader, vtkActor* actor, vtkPolyData* polyData);

  int LightComplexity = 0;
};

#endif
