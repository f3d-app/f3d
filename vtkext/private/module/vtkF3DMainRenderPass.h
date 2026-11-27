/**
 * @class   vtkF3DMainRenderPass
 * @brief   Render the 3D scene using the depth prepass texture
 *
 * @sa
 * vtkRenderPass
 */

#ifndef vtkF3DMainRenderPass_h
#define vtkF3DMainRenderPass_h

#include <vtkRenderPass.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkF3DReflectionRenderPass;
class vtkProp;
class vtkRenderer;
class vtkOpenGLFramebufferObject;
class vtkOpenGLRenderWindow;
class vtkTextureObject;

class vtkF3DMainRenderPass : public vtkRenderPass
{
public:
  static vtkF3DMainRenderPass* New();
  vtkTypeMacro(vtkF3DMainRenderPass, vtkRenderPass);

  void Render(const vtkRenderState* s) override;

  void ReleaseGraphicsResources(vtkWindow* w) override;

  ///@{
  /**
   * Set/Get the pass rendering the props into the framebuffer.
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
   * Set/Get the pass rendering the reflection of the main props, rendered before them.
   * Reflection is not rendered if null.
   */
  void SetReflectionPass(vtkF3DReflectionRenderPass* reflectionPass);
  vtkF3DReflectionRenderPass* GetReflectionPass() const
  {
    return this->ReflectionPass;
  }
  ///@}

  ///@{
  /**
   * Set/Get whether the armature props are rendered, on top of the other props.
   * Off by default.
   */
  vtkSetMacro(ArmatureVisible, bool);
  vtkGetMacro(ArmatureVisible, bool);
  ///@}

  ///@{
  /**
   * Set/Get whether raytracing is used, in which case PBR actors are converted to linear color
   * space. Off by default.
   */
  vtkSetMacro(UseRaytracing, bool);
  vtkGetMacro(UseRaytracing, bool);
  ///@}

  ///@{
  /**
   * Set/Get whether the background props are blurred, simulating a depth of field.
   * Off by default.
   */
  vtkSetMacro(UseBlurBackground, bool);
  vtkGetMacro(UseBlurBackground, bool);
  ///@}

  ///@{
  /**
   * Set/Get the size of the circle of confusion used to blur the background.
   */
  vtkSetMacro(CircleOfConfusionRadius, double);
  vtkGetMacro(CircleOfConfusionRadius, double);
  ///@}

  /**
   * Split the props of the render state in background, main and on top props, discarding the
   * others, and configure the mappers of the main props.
   * It is called by Render() but can be called earlier to access the resulting groups.
   */
  void FilterProps(const vtkRenderState* s);

  /**
   * Get the RGBA32F texture containing the color.
   */
  vtkTextureObject* GetColorTexture() const
  {
    return this->ColorTexture;
  }

  /**
   * Set the depth texture (reused from the depth pre-pass).
   */
  void SetDepthTexture(vtkTextureObject* depthTexture);

  /**
   * Set the scale applied to the rendering resolution, used for supersampling.
   */
  vtkSetMacro(ResolutionScale, double);

  vtkF3DMainRenderPass(const vtkF3DMainRenderPass&) = delete;
  void operator=(const vtkF3DMainRenderPass&) = delete;

protected:
  vtkF3DMainRenderPass();
  ~vtkF3DMainRenderPass() override;

  void InitializeResources(vtkOpenGLRenderWindow* renWin, int width, int height);
  void RenderOnTopProps(vtkRenderer* r, vtkOpenGLRenderWindow* renWin, int width, int height);

  // owned
  vtkSmartPointer<vtkTextureObject> ColorTexture;
  vtkSmartPointer<vtkTextureObject> OnTopDepthTexture;
  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;

  // reused
  vtkSmartPointer<vtkTextureObject> DepthTexture;

  vtkSmartPointer<vtkRenderPass> DelegatePass;
  vtkSmartPointer<vtkF3DReflectionRenderPass> ReflectionPass;
  vtkSmartPointer<vtkRenderPass> BackgroundPass;

  double ResolutionScale = 1.0;
  bool ArmatureVisible = false;
  bool UseRaytracing = false;
  bool UseBlurBackground = false;
  double CircleOfConfusionRadius = 20.0;

  std::vector<vtkProp*> BackgroundProps;
  std::vector<vtkProp*> MainProps;
  std::vector<vtkProp*> MainOnTopProps;
};

#endif
