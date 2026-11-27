/**
 * @class   vtkF3DReflectionRenderPass
 * @brief   Render pass baking the scene reflected by a plane into color and depth textures.
 *
 * vtkF3DReflectionRenderPass renders the props of the render state, except the grid, using its
 * delegate pass into an internal framebuffer. The active camera of the renderer is temporarily
 * replaced by a camera reflected according to the plane defined by the reflection matrix.
 *
 * @sa
 * vtkRenderPass
 */

#ifndef vtkF3DReflectionRenderPass_h
#define vtkF3DReflectionRenderPass_h

#include <vtkMatrix4x4.h>
#include <vtkRenderPass.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkCamera;
class vtkOpenGLFramebufferObject;
class vtkOpenGLRenderWindow;
class vtkProp;
class vtkTextureObject;

class vtkF3DReflectionRenderPass : public vtkRenderPass
{
public:
  static vtkF3DReflectionRenderPass* New();
  vtkTypeMacro(vtkF3DReflectionRenderPass, vtkRenderPass);

  void Render(const vtkRenderState* s) override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  ///@{
  /**
   * Set/Get the pass rendering the reflected props into the framebuffer.
   * It is expected to handle the camera, typically by being or containing a vtkCameraPass.
   */
  void SetDelegatePass(vtkRenderPass* delegate);
  vtkRenderPass* GetDelegatePass() const
  {
    return this->DelegatePass;
  }
  ///@}

  ///@{
  /**
   * Set/Get the matrix of the reflection plane. The plane normal is the second column of the
   * matrix and the plane center is its translation. Nothing is rendered without a matrix.
   */
  void SetReflectionMatrix(vtkMatrix4x4* matrix);
  vtkMatrix4x4* GetReflectionMatrix() const
  {
    return this->ReflectionMatrix;
  }
  ///@}

  /**
   * Get the RGBA16F texture containing the reflection color.
   */
  vtkTextureObject* GetColorTexture() const
  {
    return this->ColorTexture;
  }

  /**
   * Get the 32 bits depth texture of the reflection.
   */
  vtkTextureObject* GetDepthTexture() const
  {
    return this->DepthTexture;
  }

  /**
   * Set the scale applied to the rendering resolution, used for supersampling.
   */
  vtkSetMacro(ResolutionScale, double);

  vtkF3DReflectionRenderPass(const vtkF3DReflectionRenderPass&) = delete;
  void operator=(const vtkF3DReflectionRenderPass&) = delete;

protected:
  vtkF3DReflectionRenderPass();
  ~vtkF3DReflectionRenderPass() override;

  void InitializeResources(vtkOpenGLRenderWindow* renWin, int width, int height);

  void ReflectCamera(vtkCamera* originalCam, vtkCamera* reflectedCam);

  vtkSmartPointer<vtkTextureObject> ColorTexture;
  vtkSmartPointer<vtkTextureObject> DepthTexture;
  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;

  vtkSmartPointer<vtkRenderPass> DelegatePass;
  double ResolutionScale = 1.0;
  vtkSmartPointer<vtkMatrix4x4> ReflectionMatrix;

  std::vector<vtkProp*> ReflectedProps;
};

#endif
