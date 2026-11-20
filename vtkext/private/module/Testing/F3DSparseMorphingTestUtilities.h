#ifndef F3DSparseMorphingTestUtilities_h
#define F3DSparseMorphingTestUtilities_h

#include <vtkOpenGLRenderWindow.h>
#include <vtkShader.h>
#include <vtk_glad.h>

namespace F3DTest
{
inline bool SupportsSparseMorphing(vtkRenderWindow* window)
{
  auto* context = vtkOpenGLRenderWindow::SafeDownCast(window);
  if (!context || !vtkShader::IsComputeShaderSupported())
  {
    return false;
  }
  int major = 0, minor = 0;
  context->GetOpenGLVersion(major, minor);
  if (major < 4 || (major == 4 && minor < 3))
  {
    return false;
  }
  int blocks = 0;
  glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &blocks);
  return blocks >= 3;
}
}

#endif
