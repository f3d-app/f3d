/**
 * @class   vtkF3dTAAResolvePass
 * @brief   Temporal Anti-Aliasing resolve pass
 *
 * This pass is used to resolve the jittered rendering for Temporal Anti-Aliasing.
 * Adapted from https://sugulee.wordpress.com/2021/06/21/temporal-anti-aliasingtaa-tutorial/
 */

#ifndef vtkF3DTAAResolvePass_h
#define vtkF3DTAAResolvePass_h

#include "vtkImageProcessingPass.h"

#include <vtkSmartPointer.h>

#include <memory>

class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkTextureObject;

class vtkF3DTAAResolvePass : public vtkImageProcessingPass
{
public:
  static vtkF3DTAAResolvePass* New();
  vtkTypeMacro(vtkF3DTAAResolvePass, vtkImageProcessingPass);

  /**
   * Perform rendering according to a render state.
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Release graphics resources and ask components to release their own resources.
   */
  void ReleaseGraphicsResources(vtkWindow* w) override;

protected:
  void InspectCameraMovement(vtkRenderer* renderer);

private:
  vtkF3DTAAResolvePass() = default;
  ~vtkF3DTAAResolvePass() override = default;

  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  vtkSmartPointer<vtkTextureObject> ColorTexture;
  vtkSmartPointer<vtkTextureObject> HistoryTexture;

  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;

  bool HistoryInitialized = false;

  double LastPosition[3] = { 0.0, 0.0, 0.0 };
  double LastFocalPoint[3] = { 0.0, 0.0, 0.0 };
  double LastViewUp[3] = { 0.0, 0.0, 0.0 };
};
#endif
