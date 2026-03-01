/**
 * @class   vtkF3DTAAPass
 * @brief   Temporal Anti-Aliasing resolve pass
 *
 * This pass is used to perform jittering of the geometry pass and to blend the current frame
 * with the history frame to achieve Temporal Anti-Aliasing (TAA).
 * Adapted from https://sugulee.wordpress.com/2021/06/21/temporal-anti-aliasingtaa-tutorial/
 */

#ifndef vtkF3DTAAPass_h
#define vtkF3DTAAPass_h

#include "vtkImageProcessingPass.h"

#include <vtkSmartPointer.h>

#include <memory>

class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkTextureObject;

class vtkF3DTAAPass : public vtkImageProcessingPass
{
public:
  static vtkF3DTAAPass* New();
  vtkTypeMacro(vtkF3DTAAPass, vtkImageProcessingPass);

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

  /**
   * Modify shader code for jittering
   */
  bool PreReplaceShaderValues(std::string& vertexShader, std::string& geometryShader,
    std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop) override;

  /**
   * Modify shader parameters for jittering
   */
  bool SetShaderParameters(vtkShaderProgram* program, vtkAbstractMapper* mapper, vtkProp* prop,
    vtkOpenGLVertexArrayObject* VAO = nullptr) override;

private:
  vtkF3DTAAPass() = default;
  ~vtkF3DTAAPass() override = default;

  /**
   * Configure screen spaced jittering for TAA
   */
  void ConfigureJitter(int w, int h);

  /**
   * Configure Halton sequence for TAA. Valid direction values are 0 and 1. Returns a value that is
   * used for jitter
   */
  float ConfigureHaltonSequence(int direction);

  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  vtkSmartPointer<vtkTextureObject> ColorTexture;
  vtkSmartPointer<vtkTextureObject> HistoryTexture;

  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;

  int HistoryIteration = 0;
  float Jitter[2] = { 0.0f, 0.0f };
  int TaaHaltonNumerator[2] = { 0, 0 };
  int TaaHaltonDenominator[2] = { 1, 1 };
};
#endif
