#ifndef vtkF3DTAAResolvePass_h
#define vtkF3DTAAResolvePass_h

#include "vtkImageProcessingPass.h"

#include <vtkSmartPointer.h>

#include <memory>
#endif

class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkTextureObject;

class vtkF3dTAAResolvePass : public vtkImageProcessingPass
{
public:
  static vtkF3dTAAResolvePass* New();
  // vtkTypeMacro(vtkF3dTAAResolvePass, vtkImageProcessingPass);

  /**
   * Perform rendering according to a render state.
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Release graphics resources and ask components to release their own resources.
   */
  void ReleaseGraphicsResources(vtkWindow* w) override;

private:
  vtkF3dTAAResolvePass() = default;
  ~vtkF3dTAAResolvePass() override = default;

  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  vtkSmartPointer<vtkTextureObject> ColorTexture;

  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;
};