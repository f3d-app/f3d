/**
 * @class   vtkF3DExternalRenderPass
 * @brief   Render the scene with an external renderer (OSPRay, ANARI) and composite its result.
 *
 * External renderers overwrite their whole render target.
 * This pass renders the delegate in a dedicated framebuffer, then composites the result.
 *
 * @sa
 * vtkF3DMainRenderPass
 */

#ifndef vtkF3DExternalRenderPass_h
#define vtkF3DExternalRenderPass_h

#include <vtkRenderPass.h>
#include <vtkSmartPointer.h>

#include <memory>
#include <string>

class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkTextureObject;

class vtkF3DExternalRenderPass : public vtkRenderPass
{
public:
  static vtkF3DExternalRenderPass* New();
  vtkTypeMacro(vtkF3DExternalRenderPass, vtkRenderPass);

  void Render(const vtkRenderState* s) override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  enum class Backend
  {
    OSPRAY,
    ANARI
  };

  /**
   * Return true if the F3D build supports the backend.
   */
  static bool IsBackendAvailable(Backend backend);

  /**
   * Set the external renderer used.
   */
  void SetBackend(Backend backend);

  /**
   * Set the ANARI library used by the ANARI backend.
   */
  void SetAnariLibrary(const std::string& library);

  /**
   * Set whether the output is in linear color space, in sRGB otherwise.
   */
  vtkSetMacro(OutputColorIsLinear, bool);

  /**
   * Set the scale applied to the rendering resolution, used for supersampling.
   */
  vtkSetMacro(ResolutionScale, double);

  vtkF3DExternalRenderPass(const vtkF3DExternalRenderPass&) = delete;
  void operator=(const vtkF3DExternalRenderPass&) = delete;

protected:
  vtkF3DExternalRenderPass();
  ~vtkF3DExternalRenderPass() override;

  void CreateDelegatePass();

  vtkSmartPointer<vtkRenderPass> DelegatePass;
  vtkSmartPointer<vtkTextureObject> ColorTexture;
  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  std::shared_ptr<vtkOpenGLQuadHelper> QuadHelper;

  Backend CurrentBackend = Backend::OSPRAY;
  std::string AnariLibrary = "environment";
  bool OutputColorIsLinear = true;
  double ResolutionScale = 1.0;
};

#endif
