/**
 * @class   vtkF3DOverlayRenderPass
 * @brief   Implement the overlay render pass.
 *
 * Render the overlay into a fullscreen texture, then blend it with the current render pass.
 *
 * @sa
 * vtkRenderPass
 */

#ifndef vtkF3DOverlayRenderPass_h
#define vtkF3DOverlayRenderPass_h

#include <vtkFramebufferPass.h>
#include <vtkImageProcessingPass.h>
#include <vtkOpenGLQuadHelper.h>
#include <vtkSmartPointer.h>

class vtkProp;

class vtkF3DOverlayRenderPass : public vtkImageProcessingPass
{
public:
  static vtkF3DOverlayRenderPass* New();
  vtkTypeMacro(vtkF3DOverlayRenderPass, vtkImageProcessingPass);

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
  vtkF3DOverlayRenderPass(const vtkF3DOverlayRenderPass&) = delete;
  void operator=(const vtkF3DOverlayRenderPass&) = delete;

protected:
  void Initialize(const vtkRenderState* s);

  /**
   * Apply the rendered overlay frame to the current render pass.
   */
  void CompositeOverlay(const vtkRenderState* s);

  vtkSmartPointer<vtkFramebufferPass> OverlayPass;
  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  vtkSmartPointer<vtkTextureObject> ColorTexture;

  std::vector<vtkProp*> OverlayProps;

  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;

private:
  vtkF3DOverlayRenderPass() = default;
  ~vtkF3DOverlayRenderPass() override = default;
};

#endif
