/**
 * @class   vtkF3DDepthPrePass
 * @brief   Render pass writing octahedrally encoded view space normals and depth of opaque actors.
 *
 * vtkF3DDepthPrePass renders the opaque actors only into an internal framebuffer made of
 * a 32 bits depth texture and a RG8 color texture.
 * The color texture contains the view space normals, octahedrally encoded.
 *
 * The resulting textures are exposed with GetNormalTexture() and GetDepthTexture(), they are
 * consumed by vtkF3DBakeSSAOPass and the post-process pass.
 *
 * @sa
 * vtkOpenGLRenderPass
 */

#ifndef vtkF3DDepthPrePass_h
#define vtkF3DDepthPrePass_h

#include <vtkOpenGLRenderPass.h>
#include <vtkSmartPointer.h>

class vtkCameraPass;
class vtkOpenGLFramebufferObject;
class vtkOpenGLRenderWindow;
class vtkProp;
class vtkRenderPass;
class vtkTextureObject;

class vtkF3DDepthPrePass : public vtkOpenGLRenderPass
{
public:
  static vtkF3DDepthPrePass* New();
  vtkTypeMacro(vtkF3DDepthPrePass, vtkOpenGLRenderPass);

  void Render(const vtkRenderState* s) override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  /**
   * Set the pass rendering the opaque props into the framebuffer, it is wrapped into a camera pass.
   * It is typically a sequence of passes ending with a vtkOpaquePass.
   */
  void SetDelegatePass(vtkRenderPass* delegate);

  /**
   * Get the RG8 texture containing the octahedrally encoded view space normals.
   */
  vtkTextureObject* GetNormalTexture() const
  {
    return this->NormalTexture;
  }

  /**
   * Get the depth texture, owned by this pass and shared with the other passes.
   */
  vtkTextureObject* GetDepthTexture() const
  {
    return this->DepthTexture;
  }

  /**
   * Replace the fragment shader output by the octahedrally encoded normal.
   */
  bool PostReplaceShaderValues(std::string& vertexShader, std::string& geometryShader,
    std::string& fragmentShader, vtkAbstractMapper* mapper, vtkProp* prop) override;

  /**
   * Set the scale applied to the rendering resolution, used for supersampling.
   */
  vtkSetMacro(ResolutionScale, double);

  vtkF3DDepthPrePass(const vtkF3DDepthPrePass&) = delete;
  void operator=(const vtkF3DDepthPrePass&) = delete;

protected:
  vtkF3DDepthPrePass();
  ~vtkF3DDepthPrePass() override;

  void InitializeResources(vtkOpenGLRenderWindow* renWin, int width, int height);

  vtkSmartPointer<vtkTextureObject> NormalTexture;
  vtkSmartPointer<vtkTextureObject> DepthTexture;
  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;

  vtkSmartPointer<vtkRenderPass> DelegatePass;

  double ResolutionScale = 1.0;
};

#endif
