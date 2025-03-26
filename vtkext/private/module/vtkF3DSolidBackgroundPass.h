/**
 * @class   vtkF3DSolidBackgroundPass
 * @brief   Implement a post-processing blending the result of the delegate pass.
 *
 * Early passes in F3D...
 *
 * @sa
 * vtkRenderPass
 */

#ifndef vtkF3DSolidBackgroundPass_h
#define vtkF3DSolidBackgroundPass_h

#include "vtkImageProcessingPass.h"

#include <vtkSmartPointer.h>

#include <memory>

class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkTextureObject;

class vtkF3DSolidBackgroundPass : public vtkImageProcessingPass
{
public:
  static vtkF3DSolidBackgroundPass* New();
  vtkTypeMacro(vtkF3DSolidBackgroundPass, vtkImageProcessingPass);

  /**
   * Perform rendering according to a render state.
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Release graphics resources and ask components to release their own resources.
   */
  void ReleaseGraphicsResources(vtkWindow* w) override;

  /**
   * Forbidden copies.
   */
  vtkF3DSolidBackgroundPass(const vtkF3DSolidBackgroundPass&) = delete;
  void operator=(const vtkF3DSolidBackgroundPass&) = delete;

private:
  vtkF3DSolidBackgroundPass() = default;
  ~vtkF3DSolidBackgroundPass() override = default;

  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  vtkSmartPointer<vtkTextureObject> ColorTexture;

  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;
};

#endif
