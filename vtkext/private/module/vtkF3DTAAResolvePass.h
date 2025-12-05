/**
 * @class   vtkF3DTAAResolvePass
 * @brief   Temporal Anti-Aliasing resolve pass
 *
 * This pass is used to resolve the jittered rendering for Temporal Anti-Aliasing.
 * Adapted from https://sugulee.wordpress.com/2021/06/21/temporal-anti-aliasingtaa-tutorial/
 */

#ifndef vtkF3DTAAResolvePass_h
#define vtkF3DTAAResolvePass_h

#include "vtkImageProcessingPass.h"

#include <vtkMatrix4x4.h>
#include <vtkOpenGLHelper.h>
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
  void Render(const vtkRenderState* state) override;

  /**
   * Release graphics resources and ask components to release their own resources.
   */
  void ReleaseGraphicsResources(vtkWindow* window) override;

  /**
   * Reset the iterations count.
   */
  void ResetIterations()
  {
    this->HistoryIteration = 0;
  }

  bool PreReplaceShaderValues(std::string& vertexShader, std::string& geometryShader,
    std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop) override;

  bool SetShaderParameters(vtkShaderProgram* program, vtkAbstractMapper* mapper, vtkProp* prop,
    vtkOpenGLVertexArrayObject* VAO = nullptr) override;

private:
  vtkF3DTAAResolvePass() = default;
  ~vtkF3DTAAResolvePass() override = default;

  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;

  vtkSmartPointer<vtkTextureObject> ColorTexture;
  vtkSmartPointer<vtkTextureObject> DepthTexture;
  vtkSmartPointer<vtkTextureObject> HistoryTexture;
  vtkSmartPointer<vtkTextureObject> MotionVectorTexture;

  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;

  int HistoryIteration = 0;
  vtkMatrix4x4* PreviousViewProjectionMatrix = nullptr;
};
#endif
