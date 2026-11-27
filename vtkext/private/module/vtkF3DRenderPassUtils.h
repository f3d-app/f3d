/**
 * Utilities shared by the render passes of F3D.
 */

#ifndef vtkF3DRenderPassUtils_h
#define vtkF3DRenderPassUtils_h

#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLState.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtk_glad.h>

#include <cmath>
#include <optional>

namespace vtkF3DRenderPassUtils
{
/**
 * Compute the size of the framebuffers of a render pass, which is the size of the framebuffer of
 * the render state if any or the size of the renderer otherwise, scaled by the resolution scale
 * used for supersampling.
 */
inline void ComputeRenderSize(
  const vtkRenderState* s, double resolutionScale, int& width, int& height)
{
  if (s->GetFrameBuffer() == nullptr)
  {
    int x;
    int y;
    s->GetRenderer()->GetTiledSizeAndOrigin(&width, &height, &x, &y);
  }
  else
  {
    int size[2];
    s->GetWindowSize(size);
    width = size[0];
    height = size[1];
  }

  width = static_cast<int>(std::lround(width * resolutionScale));
  height = static_cast<int>(std::lround(height * resolutionScale));
}

/**
 * Depth test modes for render passes.
 * TODO: reverse-z will be implemented later
 * https://github.com/f3d-app/f3d/issues/2986
 */
enum DepthTest
{
  DepthTestNone,
  DepthTestLEqual
};

/**
 * Blend modes for render passes.
 * Define the fragment output blending behavior.
 * The resulting surface is always premultiplied alpha.
 */
enum Blend
{
  BlendNone,
  BlendStraight,
  BlendPremultiplied
};

/**
 * Options for configuring the rendering state in a render pass.
 */
struct RenderOptions
{
  DepthTest DepthTestMode = DepthTestNone;
  Blend BlendMode = BlendNone;
  std::optional<std::array<float, 4>> ClearColor = std::nullopt;
  std::optional<float> ClearDepth = std::nullopt;
  bool WriteDepth = false;
  int Width = -1;
  int Height = -1;
};

/**
 * Set the rendering options for the given OpenGL renderer.
 */
inline void SetRenderOptions(vtkOpenGLRenderer* ren, RenderOptions options)
{
  vtkOpenGLState* ostate = ren->GetState();

  // Set depth test mode
  switch (options.DepthTestMode)
  {
    case DepthTestNone:
      ostate->vtkglDisable(GL_DEPTH_TEST);
      break;
    case DepthTestLEqual:
      ostate->vtkglEnable(GL_DEPTH_TEST);
      ostate->vtkglDepthFunc(GL_LEQUAL);
      break;
  }

  // Set blend mode
  switch (options.BlendMode)
  {
    case BlendNone:
      ostate->vtkglDisable(GL_BLEND);
      break;
    case BlendStraight:
      ostate->vtkglEnable(GL_BLEND);
      ostate->vtkglBlendFuncSeparate(
        GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      break;
    case BlendPremultiplied:
      ostate->vtkglEnable(GL_BLEND);
      ostate->vtkglBlendFuncSeparate(
        GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      break;
  }

  // Must be set before clearing
  ostate->vtkglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  ostate->vtkglDepthMask(options.WriteDepth ? GL_TRUE : GL_FALSE);
  ostate->vtkglViewport(0, 0, options.Width, options.Height);
  ostate->vtkglDisable(GL_SCISSOR_TEST);
  ostate->vtkglDisable(GL_STENCIL_TEST);

  unsigned int clearMask = 0;

  if (options.ClearColor.has_value())
  {
    ostate->vtkglClearColor(options.ClearColor.value()[0], options.ClearColor.value()[1],
      options.ClearColor.value()[2], options.ClearColor.value()[3]);
    clearMask |= GL_COLOR_BUFFER_BIT;
  }
  if (options.ClearDepth.has_value())
  {
    ostate->vtkglClearDepth(options.ClearDepth.value());
    clearMask |= GL_DEPTH_BUFFER_BIT;
  }

  if (clearMask != 0)
  {
    ostate->vtkglClear(clearMask);
  }
}

}

#endif
