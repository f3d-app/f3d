/**
 * @class   vtkF3DFrameCapture
 * @brief   Frame capture from a texture.
 *
 * vtkF3DFrameCapture is a helper class to capture a texture and convert it to YUV planes.
 * It is used to capture frames for video encoding.
 * The conversion follows the Y'UV color space standard as described in
 * https://en.wikipedia.org/wiki/Y%E2%80%B2UV
 * This color space is commonly used in video compression and expected input of encoders.
 */

#ifndef vtkF3DFrameCapture_h
#define vtkF3DFrameCapture_h

#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkTextureObject.h>

#include <memory>

class vtkOpenGLRenderer;

class vtkF3DFrameCapture : public vtkObject
{
public:
  static vtkF3DFrameCapture* New();
  vtkTypeMacro(vtkF3DFrameCapture, vtkObject);

  /**
   * Set the input texture to capture. The texture must be a 2D texture with RGBA format.
   */
  vtkSetSmartPointerMacro(InputTexture, vtkTextureObject);

  /**
   * Capture the current texture and fill the provided buffers with Y, U, and V planes.
   * The buffers must be allocated with the correct size:
   * - Y plane: width * height bytes
   * - U plane: (width / 2) * (height / 2) bytes
   * - V plane: (width / 2) * (height / 2) bytes
   */
  void Capture(vtkOpenGLRenderer* ren, std::byte* y, std::byte* u, std::byte* v);

  /**
   * Release graphics resources.
   */
  void ReleaseGraphicsResources(vtkWindow* win);

  vtkF3DFrameCapture(const vtkF3DFrameCapture&) = delete;
  void operator=(const vtkF3DFrameCapture&) = delete;

protected:
  vtkF3DFrameCapture();
  ~vtkF3DFrameCapture() override;

private:
  struct Internals;
  std::unique_ptr<Internals> Pimpl;

  /**
   * Render the input texture to YUV planes.
   */
  void RenderYUV(vtkOpenGLRenderer* ren);

  vtkSmartPointer<vtkTextureObject> InputTexture = nullptr;
};

#endif
