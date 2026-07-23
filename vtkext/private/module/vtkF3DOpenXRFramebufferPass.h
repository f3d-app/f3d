#ifndef vtkF3DOpenXRFramebufferPass_h
#define vtkF3DOpenXRFramebufferPass_h

#include "vtkFramebufferPass.h"

class vtkF3DOpenXRFramebufferPass : public vtkFramebufferPass
{
public:
  static vtkF3DOpenXRFramebufferPass* New();
  vtkTypeMacro(vtkF3DOpenXRFramebufferPass, vtkFramebufferPass);

  // modified from vtkFramebufferPass to adapt the camera when rendering in OpenXR mode
  void RenderDelegate(const vtkRenderState* s, int width, int height, int newWidth, int newHeight,
    vtkOpenGLFramebufferObject* fbo, vtkTextureObject* colortarget,
    vtkTextureObject* depthtarget) override;

private:
  vtkF3DOpenXRFramebufferPass() = default;
  ~vtkF3DOpenXRFramebufferPass() override = default;
};
#endif
