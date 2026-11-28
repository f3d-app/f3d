/**
 * @class   vtkF3DHookShaderRenderPass
 * @brief   Base class for the render passes modifying the shaders of the props they render.
 *
 * vtkF3DHookShaderRenderPass registers itself on the props of the render state, renders its
 * delegate pass and unregisters itself. While registered, the shader replacement methods of the
 * pass (PreReplaceShaderValues, PostReplaceShaderValues) are applied by the mappers of the props.
 *
 * @sa
 * vtkOpenGLRenderPass
 */

#ifndef vtkF3DHookShaderRenderPass_h
#define vtkF3DHookShaderRenderPass_h

#include <vtkOpenGLRenderPass.h>
#include <vtkSmartPointer.h>

class vtkF3DHookShaderRenderPass : public vtkOpenGLRenderPass
{
public:
  static vtkF3DHookShaderRenderPass* New();
  vtkTypeMacro(vtkF3DHookShaderRenderPass, vtkOpenGLRenderPass);

  /**
   * Register the pass on the props, render the delegate pass and unregister the pass.
   */
  void Render(const vtkRenderState* s) override;

  ///@{
  /**
   * Set the pass rendering the props with the modified shaders.
   */
  void SetDelegatePass(vtkRenderPass* delegate);
  ///@}

  vtkF3DHookShaderRenderPass(const vtkF3DHookShaderRenderPass&) = delete;
  void operator=(const vtkF3DHookShaderRenderPass&) = delete;

protected:
  vtkF3DHookShaderRenderPass() = default;
  ~vtkF3DHookShaderRenderPass() override = default;

  vtkSmartPointer<vtkRenderPass> DelegatePass;
};

#endif
