#include "vtkF3DOpenGLGridMapper.h"

#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLRenderPass.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLVertexArrayObject.h>
#include <vtkOpenGLVertexBufferObjectGroup.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkVersion.h>

vtkStandardNewMacro(vtkF3DOpenGLGridMapper);

//----------------------------------------------------------------------------
vtkF3DOpenGLGridMapper::vtkF3DOpenGLGridMapper()
{
  this->SetNumberOfInputPorts(0);
  this->StaticOn();
}

//----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FadeDistance: " << this->FadeDistance << "\n";
  os << indent << "UnitSquare: " << this->UnitSquare << "\n";
  os << indent << "MinorSquare: " << this->MinorSquares << "\n";
  os << indent << "UpIndex: " << this->UpIndex << "\n";
}

//----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::ReplaceShaderValues(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  this->ReplaceShaderRenderPass(shaders, ren, actor, true);

  std::string VSSource = shaders[vtkShader::Vertex]->GetSource();
  std::string FSSource = shaders[vtkShader::Fragment]->GetSource();

  const std::string axes = this->UpIndex == 0 ? "zyx" : this->UpIndex == 1 ? "xzy" : "xyz";

  // clang-format off
  vtkShaderProgram::Substitute(VSSource, "//VTK::PositionVC::Dec",
    "uniform float fadeDist = 100.0;\n"
    "out vec2 gridCoord;\n"
  );
  vtkShaderProgram::Substitute(VSSource, "//VTK::PositionVC::Impl",
    "gridCoord = vertexMC.xy * fadeDist;\n"
    "gl_Position = MCDCMatrix * vec4(vertexMC." + axes + " * fadeDist, 1.0);\n"
  );
  
  vtkShaderProgram::Substitute(FSSource, "//VTK::CustomUniforms::Dec",
    "uniform float fadeDist = 100.0;\n"
    "uniform float unitSquare = 10.0;\n"
    "uniform int minorSquares = 1;\n"
    "uniform float axesLineWidth = 1.0;\n"
    "uniform float gridLineWidth = 1.0;\n"
    "uniform float minorOpacity = 0.5;\n"
    "uniform float lineAntialias = 1.0;\n"
    "uniform vec4 axis1Color = vec4(1., 1., 1., 1.);\n"
    "uniform vec4 axis2Color = vec4(1., 1., 1., 1.);\n"
    "in vec2 gridCoord;\n"

    "float antialias(float dist, float linewidth){\n"
    "  float aa = lineAntialias;\n"
    "  float lw = max(linewidth, 1.0) / 2;\n"
    "  float alpha = min(linewidth, 1.0);\n"
    "  float d = dist - lw;\n"
    "  return d < .0 ? alpha\n"
    "       : d < aa ? (1 - d / aa) * alpha\n"
    "       : 0.0;\n"
    "}\n"
  );
  vtkShaderProgram::Substitute(FSSource, "//VTK::UniformFlow::Impl",
    "  vec2 majorCoord = gridCoord / unitSquare;\n"
    "  vec2 minorCoord = majorCoord * minorSquares;\n"
    "  vec2 majorGrid = abs(fract(majorCoord - 0.5) - 0.5) / fwidth(majorCoord);\n"
    "  vec2 minorGrid = abs(fract(minorCoord - 0.5) - 0.5) / fwidth(minorCoord);\n"
  );
  vtkShaderProgram::Substitute(FSSource, "//VTK::Color::Impl",
    "  float majorAlpha = antialias(min(majorGrid.x, majorGrid.y), gridLineWidth);\n"
    "  float minorAlpha = antialias(min(minorGrid.x, minorGrid.y), gridLineWidth);\n"
    "  float zoomFadeFactor = 1.0 - clamp(fwidth(majorCoord.x / unitSquare * fadeDist), 0.0, 1.0);"
    "  float alpha = max(majorAlpha, minorAlpha * minorOpacity * zoomFadeFactor);\n"
    "  vec4 color = vec4(diffuseColorUniform, alpha);\n"

    "  float axis1Weight = abs(majorCoord.y) < 0.5 ? antialias(majorGrid.y, axesLineWidth) : 0.0;\n"
    "  float axis2Weight = abs(majorCoord.x) < 0.5 ? antialias(majorGrid.x, axesLineWidth) : 0.0;\n"
    "  color = mix(color, axis2Color, axis2Weight);\n"
    "  color = mix(color, axis1Color, axis1Weight);\n"

    "  float sqDist = unitSquare * unitSquare * dot(majorCoord, majorCoord);\n"
    "  float radialFadeFactor = 1.0 - sqDist / (fadeDist * fadeDist);\n"
    "  color.w *= radialFadeFactor;\n"

    "  gl_FragData[0] = color;\n"
  );
  // clang-format on

  shaders[vtkShader::Fragment]->SetSource(FSSource);
  shaders[vtkShader::Vertex]->SetSource(VSSource);

  // add camera uniforms declaration
  this->ReplaceShaderPositionVC(shaders, ren, actor);

  // add color uniforms declaration
  this->ReplaceShaderColor(shaders, ren, actor);

  // for depth peeling
  this->ReplaceShaderRenderPass(shaders, ren, actor, false);
}

//----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::SetMapperShaderParameters(
  vtkOpenGLHelper& cellBO, vtkRenderer* vtkNotUsed(ren), vtkActor* actor)
{
  if (this->VBOs->GetMTime() > cellBO.AttributeUpdateTime ||
    cellBO.ShaderSourceTime > cellBO.AttributeUpdateTime)
  {
    cellBO.VAO->Bind();
    this->VBOs->AddAllAttributesToVAO(cellBO.Program, cellBO.VAO);

    cellBO.AttributeUpdateTime.Modified();
  }

  // Handle render pass setup:
  vtkInformation* info = actor->GetPropertyKeys();
  if (info && info->Has(vtkOpenGLRenderPass::RenderPasses()))
  {
    int numRenderPasses = info->Length(vtkOpenGLRenderPass::RenderPasses());
    for (int i = 0; i < numRenderPasses; ++i)
    {
      vtkObjectBase* rpBase = info->Get(vtkOpenGLRenderPass::RenderPasses(), i);
      vtkOpenGLRenderPass* rp = vtkOpenGLRenderPass::SafeDownCast(rpBase);
      if (rp && !rp->SetShaderParameters(cellBO.Program, this, actor, cellBO.VAO))
      {
        vtkErrorMacro(
          "RenderPass::SetShaderParameters failed for renderpass: " << rp->GetClassName());
      }
    }
  }

  cellBO.Program->SetUniformf("fadeDist", this->FadeDistance);
  cellBO.Program->SetUniformf("unitSquare", this->UnitSquare);
  cellBO.Program->SetUniformi("minorSquares", this->MinorSquares);
  cellBO.Program->SetUniformf("axesLineWidth", 0.8);
  cellBO.Program->SetUniformf("gridLineWidth", 0.6);
  cellBO.Program->SetUniformf("minorOpacity", 0.5);
  cellBO.Program->SetUniformf("lineAntialias", 1);

  const float xColor[4] = { 1, 0, 0, 1 };
  const float yColor[4] = { 0, 1, 0, 1 };
  const float zColor[4] = { 0, 0, 1, 1 };
  switch (this->UpIndex)
  {
    case 0:
      cellBO.Program->SetUniform4f("axis1Color", zColor);
      cellBO.Program->SetUniform4f("axis2Color", yColor);
      break;
    case 1:
      cellBO.Program->SetUniform4f("axis1Color", xColor);
      cellBO.Program->SetUniform4f("axis2Color", zColor);
      break;
    case 2:
    default:
      cellBO.Program->SetUniform4f("axis1Color", xColor);
      cellBO.Program->SetUniform4f("axis2Color", yColor);
      break;
  }
}

//----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::BuildBufferObjects(vtkRenderer* ren, vtkActor* vtkNotUsed(act))
{
  vtkNew<vtkFloatArray> infinitePlane;
  infinitePlane->SetNumberOfComponents(2);
  infinitePlane->SetNumberOfTuples(4);

  float corner1[] = { -1, -1 };
  float corner2[] = { +1, -1 };
  float corner3[] = { -1, +1 };
  float corner4[] = { +1, +1 };

  infinitePlane->SetTuple(0, corner1);
  infinitePlane->SetTuple(1, corner2);
  infinitePlane->SetTuple(2, corner3);
  infinitePlane->SetTuple(3, corner4);

  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow());
  vtkOpenGLVertexBufferObjectCache* cache = renWin->GetVBOCache();

  this->VBOs->CacheDataArray("vertexMC", infinitePlane, cache, VTK_FLOAT);
  this->VBOs->BuildAllVBOs(cache);

  vtkOpenGLCheckErrorMacro("failed after BuildBufferObjects");

  this->VBOBuildTime.Modified();
}

//-----------------------------------------------------------------------------
double* vtkF3DOpenGLGridMapper::GetBounds()
{
  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = -this->FadeDistance;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = +this->FadeDistance;
  return this->Bounds;
}

//-----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::RenderPiece(vtkRenderer* ren, vtkActor* actor)
{
  // Update/build/etc the shader.
  this->UpdateBufferObjects(ren, actor);
  this->UpdateShaders(this->Primitives[PrimitivePoints], ren, actor);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

//-----------------------------------------------------------------------------
bool vtkF3DOpenGLGridMapper::GetNeedToRebuildShaders(
  vtkOpenGLHelper& cellBO, vtkRenderer* vtkNotUsed(ren), vtkActor* act)
{
// Complete GetRenderPassStageMTime needs in
// https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7933
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210506)
  vtkMTimeType renderPassMTime = this->GetRenderPassStageMTime(act, &cellBO);
#else
  vtkMTimeType renderPassMTime = this->GetRenderPassStageMTime(act);
#endif
  return cellBO.Program == nullptr || cellBO.ShaderSourceTime < renderPassMTime;
}
