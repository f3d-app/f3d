/**
 * @class   vtkF3DHexagonalBokehBlurPass
 * @brief   Implement a two-passes hexagonal Bokeh blur.
 *
 * This pass is used to blur the background and simulate a depth of field.
 * Adapted from "Advances in Real-Time Rendering", Siggraph 2011
 * https://colinbarrebrisebois.com/2017/04/18/hexagonal-bokeh-blur-revisited-part-1-basic-3-pass-version/
 *
 */

#ifndef vtkF3DHexagonalBokehBlurPass_h
#define vtkF3DHexagonalBokehBlurPass_h

#include "vtkImageProcessingPass.h"

#include "vtkSmartPointer.h"

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
  vtkGetMacro(CircleOfConfusionRadius, float);
  vtkSetMacro(CircleOfConfusionRadius, float);
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
   * Initialize graphics resources.
   */
  void InitializeGraphicsResources(vtkOpenGLRenderWindow* renWin, int width, int height);

  /**
   * Render delegate pass.
   */
  void RenderDelegate(const vtkRenderState* s, int width, int height);

  /**
   * First pass: blur vertically and diagonally the input to produce two outputs.
   */
  void RenderDirectionalBlur(vtkOpenGLRenderWindow* renWin, int width, int height);

  /**
   * Second pass: Combine and blur the two inputs to get the final result.
   */
  void RenderRhomboidBlur(vtkOpenGLRenderWindow* renWin, int width, int height);

  /**
   * Graphics resources.
   */
  vtkSmartPointer<vtkOpenGLFramebufferObject> FrameBufferObject;
  vtkSmartPointer<vtkTextureObject> VerticalBlurTexture;
  vtkSmartPointer<vtkTextureObject> DiagonalBlurTexture;
  vtkSmartPointer<vtkTextureObject> BackgroundTexture;

  vtkOpenGLQuadHelper* BlurQuadHelper = nullptr;
  vtkOpenGLQuadHelper* RhomboidQuadHelper = nullptr;

  float CircleOfConfusionRadius = 20.f;

private:
  vtkF3DHexagonalBokehBlurPass(const vtkF3DHexagonalBokehBlurPass&) = delete;
  void operator=(const vtkF3DHexagonalBokehBlurPass&) = delete;
};

#endif
