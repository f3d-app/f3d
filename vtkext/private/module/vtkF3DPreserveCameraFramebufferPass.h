/**
 * @file vtkF3DPreserveCameraFramebufferPass.h
 * @brief Framework to preserve the camera when rendering into a framebuffer.
 *
 * Required as vtkFramebufferPass does not adapt the camera
 *
 */
#ifndef vtkF3DPreserveCameraFramebufferPass_h
#define vtkF3DPreserveCameraFramebufferPass_h

#include "vtkFramebufferPass.h"

class vtkF3DPreserveCameraFramebufferPass : public vtkFramebufferPass
{
public:
  static vtkF3DPreserveCameraFramebufferPass* New();
  vtkTypeMacro(vtkF3DPreserveCameraFramebufferPass, vtkFramebufferPass);

  // modified from vtkFramebufferPass to disable the camera rendering into a new width and height
  void RenderDelegate(const vtkRenderState* s, int width, int height, int newWidth, int newHeight,
    vtkOpenGLFramebufferObject* fbo, vtkTextureObject* colortarget,
    vtkTextureObject* depthtarget) override;

private:
  vtkF3DPreserveCameraFramebufferPass() = default;
  ~vtkF3DPreserveCameraFramebufferPass() override = default;
};
#endif
