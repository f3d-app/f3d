/**
 * @class   vtkF3DPostprocessRenderPass
 * @brief   Render pass drawing the rendered textures onto the output framebuffer.
 *
 * vtkF3DPostprocessRenderPass draws a full screen quad sampling the color texture on the
 * framebuffer of the render state and perform sRGB conversion and optionally tone mapping.
 * The depth, normal and colormap textures are made available to the shader.
 * The final shader can be customized using the UserShader property.
 *
 * @sa
 * vtkRenderPass
 */

#ifndef vtkF3DPostprocessRenderPass_h
#define vtkF3DPostprocessRenderPass_h

#include <vtkRenderPass.h>
#include <vtkSmartPointer.h>
#include <vtkTimeStamp.h>

#include <memory>
#include <string>

class vtkDiscretizableColorTransferFunction;
class vtkOpenGLQuadHelper;
class vtkOpenGLRenderWindow;
class vtkTextureObject;

class vtkF3DPostprocessRenderPass : public vtkRenderPass
{
public:
  static vtkF3DPostprocessRenderPass* New();
  vtkTypeMacro(vtkF3DPostprocessRenderPass, vtkRenderPass);

  void Render(const vtkRenderState* s) override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  vtkSetMacro(ColorInputIsLinear, bool);

  /**
   * Set the resolution scale of the input textures, if above 1 the color is downsampled (SSAA).
   */
  vtkSetMacro(ResolutionScale, double);
  vtkSetMacro(UserShader, std::string);
  vtkSetMacro(UseToneMapping, bool);
  vtkGetMacro(UseToneMapping, bool);

  /**
   * Set the color texture to draw. Nothing is rendered without it.
   */
  void SetColorTexture(vtkTextureObject* texture)
  {
    this->ColorTexture = texture;
  }

  /**
   * Set the optional depth texture.
   */
  void SetDepthTexture(vtkTextureObject* texture)
  {
    this->DepthTexture = texture;
  }

  /**
   * Set the optional normal texture.
   */
  void SetNormalTexture(vtkTextureObject* texture)
  {
    this->NormalTexture = texture;
  }

  /**
   * Set/Get the optional color map available to the shader as a 256x1 texture, sampled using
   * values in the [0, 1] range.
   * If not set, the texture is a black to white ramp.
   */
  void SetColorMap(vtkDiscretizableColorTransferFunction* colorMap);

  vtkF3DPostprocessRenderPass(const vtkF3DPostprocessRenderPass&) = delete;
  void operator=(const vtkF3DPostprocessRenderPass&) = delete;

protected:
  vtkF3DPostprocessRenderPass();
  ~vtkF3DPostprocessRenderPass() override;

  // the textures are owned by other passes
  vtkTextureObject* ColorTexture = nullptr;
  vtkTextureObject* DepthTexture = nullptr;
  vtkTextureObject* NormalTexture = nullptr;

  void UpdateColorMapTexture(vtkOpenGLRenderWindow* renWin);

  vtkSmartPointer<vtkDiscretizableColorTransferFunction> ColorMap;
  vtkSmartPointer<vtkTextureObject> ColorMapTexture;
  vtkTimeStamp ColorMapBuildTime;

  bool ColorInputIsLinear = true;
  bool UseToneMapping = false;
  std::string UserShader;
  double ResolutionScale = 1.0;

  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;
};

#endif
