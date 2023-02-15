/**
 * @class   vtkF3DHexagonalBokehBlurPass
 * @brief   TODO
 */

#ifndef vtkF3DHexagonalBokehBlurPass_h
#define vtkF3DHexagonalBokehBlurPass_h

#include "vtkImageProcessingPass.h"

class vtkOpenGLFramebufferObject;
class vtkOpenGLQuadHelper;
class vtkOpenGLRenderWindow;
class vtkTextureObject;

class vtkF3DHexagonalBokehBlurPass : public vtkImageProcessingPass
{
public:
  static vtkF3DHexagonalBokehBlurPass* New();
  vtkTypeMacro(vtkF3DHexagonalBokehBlurPass, vtkImageProcessingPass);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Perform rendering according to a render state.
   */
  void Render(const vtkRenderState* s) override;

  /**
   * Release graphics resources and ask components to release their own
   * resources.
   */
  void ReleaseGraphicsResources(vtkWindow* w) override;

  ///@{
  /**
   * Get/Set the size of the circle of confusion.
   */
  vtkGetMacro(CircleOfConfusion, float);
  vtkSetMacro(CircleOfConfusion, float);
  ///@}

protected:
  /**
   * Default constructor.
   */
  vtkF3DHexagonalBokehBlurPass();

  /**
   * Destructor.
   */
  ~vtkF3DHexagonalBokehBlurPass() override;

  /**
   * Intialize graphics resources.
   */
  void InitializeGraphicsResources(vtkOpenGLRenderWindow* renWin, int w, int h);

  void RenderDelegate(const vtkRenderState* s, int w, int h);
  void RenderDirectionalBlur(vtkOpenGLRenderWindow* renWin, int w, int h);
  void RenderRhomboidBlur(vtkOpenGLRenderWindow* renWin, int w, int h);

  /**
   * Graphics resources.
   */
  vtkOpenGLFramebufferObject* FrameBufferObject = nullptr;
  vtkTextureObject* VerticalBlurTexture = nullptr;
  vtkTextureObject* DiagonalBlurTexture = nullptr;
  vtkTextureObject* BackgroundTexture = nullptr;

  vtkOpenGLQuadHelper* BlurQuadHelper = nullptr;
  vtkOpenGLQuadHelper* RhomboidQuadHelper = nullptr;

  float CircleOfConfusion = 20.f;

private:
  vtkF3DHexagonalBokehBlurPass(const vtkF3DHexagonalBokehBlurPass&) = delete;
  void operator=(const vtkF3DHexagonalBokehBlurPass&) = delete;
};

#endif
