#ifndef vtkF3DDepthRenderPass_h
#define vtkF3DDepthRenderPass_h

#include "vtkImageProcessingPass.h"

#include <vtkDepthImageProcessingPass.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkSmartPointer.h>

#include <memory>

class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkTextureObject;

class vtkF3DDisplayDepthRenderPass : public vtkDepthImageProcessingPass
{
public:
  static vtkF3DDisplayDepthRenderPass* New();
  vtkTypeMacro(vtkF3DDisplayDepthRenderPass, vtkDepthImageProcessingPass);
  void Render(const vtkRenderState* state) override;
  void ReleaseGraphicsResources(vtkWindow* window) override;

  void SetColorMap(vtkDiscretizableColorTransferFunction* colorMap);

private:
  vtkF3DDisplayDepthRenderPass() = default;
  ~vtkF3DDisplayDepthRenderPass() override = default;

  void InitializeResources(vtkOpenGLRenderWindow* renWin, int w, int h);

  vtkSmartPointer<vtkDiscretizableColorTransferFunction> ColorMap = nullptr;
  vtkTimeStamp ColorMapBuildTime;

  vtkSmartPointer<vtkTextureObject> ColorTexture;
  vtkSmartPointer<vtkTextureObject> DepthTexture;
  vtkSmartPointer<vtkTextureObject> ColorMapTexture;
  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;
};
#endif