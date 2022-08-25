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
  os << indent << "UpIndex: " << this->UpIndex << "\n";
}

//----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::ReplaceShaderValues(
  std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
  this->ReplaceShaderRenderPass(shaders, ren, actor, true);

  std::string VSSource = shaders[vtkShader::Vertex]->GetSource();
  std::string FSSource = shaders[vtkShader::Fragment]->GetSource();

  vtkShaderProgram::Substitute(
    VSSource, "//VTK::PositionVC::Dec", "out vec4 positionMCVSOutput;\n");

  vtkShaderProgram::Substitute(FSSource, "//VTK::PositionVC::Dec",
    "in vec4 positionMCVSOutput;\n"
    "uniform float fadeDist;\n"
    "uniform float unitSquare;\n");

  std::string posImpl;
  std::string uniformImpl;

  std::string colorImpl =
    "  float line = min(grid.x, grid.y);\n"
    "  float dist2 = unitSquare * unitSquare * (coord.x * coord.x + coord.y * coord.y);\n"
    "  float opacity = (1.0 - min(line, 1.0)) * (1.0 - dist2 / (fadeDist * fadeDist));\n"
    "  vec3 color = diffuseColorUniform;\n";

  switch (this->UpIndex)
  {
    case 0:
      posImpl += "positionMCVSOutput = vec4(0.0, vertexMC.x, vertexMC.y, 1.0);\n";
      uniformImpl +=
        "  vec2 coord = positionMCVSOutput.yz / (unitSquare * positionMCVSOutput.w);\n";
      colorImpl += "  if (abs(coord.x) < 0.1 && grid.y != line) color = vec3(0.0, 0.0, 1.0);\n"
                   "  if (abs(coord.y) < 0.1 && grid.x != line) color = vec3(0.0, 1.0, 0.0);\n";
      break;
    case 1:
      posImpl += "positionMCVSOutput = vec4(vertexMC.x, 0.0, vertexMC.y, 1.0);\n";
      uniformImpl +=
        "  vec2 coord = positionMCVSOutput.xz / (unitSquare * positionMCVSOutput.w);\n";
      colorImpl += "  if (abs(coord.x) < 0.1 && grid.y != line) color = vec3(0.0, 0.0, 1.0);\n"
                   "  if (abs(coord.y) < 0.1 && grid.x != line) color = vec3(1.0, 0.0, 0.0);\n";
      break;
    case 2:
      posImpl += "positionMCVSOutput = vec4(vertexMC.x, vertexMC.y, 0.0, 1.0);\n";
      uniformImpl +=
        "  vec2 coord = positionMCVSOutput.xy / (unitSquare * positionMCVSOutput.w);\n";
      colorImpl += "  if (abs(coord.x) < 0.1 && grid.y != line) color = vec3(0.0, 1.0, 0.0);\n"
                   "  if (abs(coord.y) < 0.1 && grid.x != line) color = vec3(1.0, 0.0, 0.0);\n";
      break;
  }
  posImpl += "gl_Position = MCDCMatrix * positionMCVSOutput;\n";
  uniformImpl += "  vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);\n";
  colorImpl += "  gl_FragData[0] = vec4(color, opacity);\n";

  vtkShaderProgram::Substitute(VSSource, "//VTK::PositionVC::Impl", posImpl);
  vtkShaderProgram::Substitute(FSSource, "  //VTK::UniformFlow::Impl", uniformImpl);
  vtkShaderProgram::Substitute(FSSource, "  //VTK::Color::Impl", colorImpl);

  shaders[vtkShader::Vertex]->SetSource(VSSource);
  shaders[vtkShader::Fragment]->SetSource(FSSource);

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
}

//----------------------------------------------------------------------------
void vtkF3DOpenGLGridMapper::BuildBufferObjects(vtkRenderer* ren, vtkActor* vtkNotUsed(act))
{
  vtkNew<vtkFloatArray> infinitePlane;
  infinitePlane->SetNumberOfComponents(2);
  infinitePlane->SetNumberOfTuples(4);

  float d = static_cast<float>(this->FadeDistance);
  float corner1[] = { -d, -d };
  float corner2[] = { d, -d };
  float corner3[] = { -d, d };
  float corner4[] = { d, d };

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
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = this->FadeDistance;
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
