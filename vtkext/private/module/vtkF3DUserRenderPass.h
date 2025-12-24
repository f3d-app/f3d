/**
 * @class   vtkF3DUserRenderPass
 * @brief   Implement a post-processing using user string.
 *
 * Full screen quad render pass used to run a user provided GLSL function.
 * The function code should be set with SetUserShader and requires the following signature:
 *
 * @code{glsl}
 * vec4 pixel(vec2 uv)
 * {
 *   vec4 outColor = ...;
 *   return outColor;
 * }
 * @endcode
 *
 * The input argument is the position of the pixel in normalized coordinates.
 * (0, 0) is the bottom left corner pixel and (1, 1) is the upper right pixel.
 * The first component is the horizontal direction.
 * The returned value is the RGBA output pixel.
 * This function is executed on the GPU, in parallel, for every single pixel of the window.
 *
 * A few uniform variables (same value for each shader invocation) is available to use:
 * - `sampler2d source`: a texture containing the result of F3D rendering pipeline
 * - `ivec2 resolution`: a integer pair containing the resolution (in pixels) of the window
 *
 * @sa
 * vtkRenderPass
 */

#ifndef vtkF3DUserRenderPass_h
#define vtkF3DUserRenderPass_h

#include "vtkImageProcessingPass.h"

#include <vtkSmartPointer.h>

#include <memory>

class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkTextureObject;

class vtkF3DUserRenderPass : public vtkImageProcessingPass
{
public:
  static vtkF3DUserRenderPass* New();
  vtkTypeMacro(vtkF3DUserRenderPass, vtkImageProcessingPass);

  /**
   * Perform rendering according to a render state.
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Release graphics resources and ask components to release their own resources.
   */
  void ReleaseGraphicsResources(vtkWindow* w) override;

  /**
   * Set the user shader code.
   */
  vtkSetMacro(UserShader, std::string);

  /**
   * Forbidden copies.
   */
  vtkF3DUserRenderPass(const vtkF3DUserRenderPass&) = delete;
  void operator=(const vtkF3DUserRenderPass&) = delete;

private:
  vtkF3DUserRenderPass() = default;
  ~vtkF3DUserRenderPass() override = default;

  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  vtkSmartPointer<vtkTextureObject> ColorTexture;

  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;
  std::string UserShader;
};

#endif
