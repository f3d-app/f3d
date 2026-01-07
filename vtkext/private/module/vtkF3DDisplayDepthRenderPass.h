#ifndef vtkF3DDepthRenderPass_h
#define vtkF3DDepthRenderPass_h

#include "vtkImageProcessingPass.h"

#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkSmartPointer.h>

#include <memory>

class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkTextureObject;

class vtkF3DDisplayDepthRenderPass : public vtkImageProcessingPass
{
public:
  static vtkF3DDisplayDepthRenderPass* New();
  vtkTypeMacro(vtkF3DDisplayDepthRenderPass, vtkImageProcessingPass);
  void Render(const vtkRenderState* state) override;
  void ReleaseGraphicsResources(vtkWindow* window) override;
  bool PreReplaceShaderValues(std::string& vertexShader, std::string& geometryShader,
    std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop) override;
  void SetColorMap(vtkDiscretizableColorTransferFunction* colorMap);

private:
  vtkF3DDisplayDepthRenderPass() = default;
  ~vtkF3DDisplayDepthRenderPass() override = default;

  vtkDiscretizableColorTransferFunction* ColorMap = nullptr;
  bool ColorMapDirty = true;

  vtkSmartPointer<vtkTextureObject> DepthTexture;
  vtkSmartPointer<vtkTextureObject> ColorMapTexture;
  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;
};
#endif