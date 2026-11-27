#include "vtkF3DPostprocessRenderPass.h"

#include "vtkF3DPostprocessFS.h"
#include "vtkF3DRenderPassUtils.h"
#include "vtkF3DRenderer.h"

#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLQuadHelper.h>
#include <vtkOpenGLRenderUtilities.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkTextureObject.h>

#include <array>
#include <sstream>

vtkStandardNewMacro(vtkF3DPostprocessRenderPass);

//------------------------------------------------------------------------------
vtkF3DPostprocessRenderPass::vtkF3DPostprocessRenderPass() = default;

//------------------------------------------------------------------------------
vtkF3DPostprocessRenderPass::~vtkF3DPostprocessRenderPass() = default;

//------------------------------------------------------------------------------
void vtkF3DPostprocessRenderPass::SetColorMap(vtkDiscretizableColorTransferFunction* colorMap)
{
  if (this->ColorMap != colorMap)
  {
    this->ColorMap = colorMap;
    this->ColorMapTextureValid = false;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
void vtkF3DPostprocessRenderPass::UpdateColorMapTexture(vtkOpenGLRenderWindow* renWin)
{
  if (!this->ColorMapTexture)
  {
    this->ColorMapTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->ColorMapTexture->SetContext(renWin);
    this->ColorMapTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->ColorMapTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->ColorMapTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->ColorMapTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->ColorMapTexture->SetFormat(GL_RGB);
    this->ColorMapTexture->SetInternalFormat(GL_RGB16F);
    this->ColorMapTexture->SetDataType(GL_FLOAT);
  }

  const bool upToDate = this->ColorMapTexture->GetHandle() && this->ColorMapTextureValid &&
    (!this->ColorMap || this->ColorMapBuildTime.GetMTime() > this->ColorMap->GetMTime());
  if (upToDate)
  {
    return;
  }

  constexpr int tableSize = 256;
  std::array<float, tableSize * 3> table;
  if (this->ColorMap)
  {
    double range[2];
    this->ColorMap->GetRange(range);
    this->ColorMap->GetTable(range[0], range[1], tableSize, table.data());
  }
  else
  {
    for (int i = 0; i < tableSize; i++)
    {
      table[3 * i] = table[3 * i + 1] = table[3 * i + 2] = i / static_cast<float>(tableSize - 1);
    }
  }

  this->ColorMapTexture->Create2DFromRaw(tableSize, 1, 3, VTK_FLOAT, table.data());
  this->ColorMapTextureValid = true;
  this->ColorMapBuildTime.Modified();
}

//------------------------------------------------------------------------------
void vtkF3DPostprocessRenderPass::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);

  if (this->QuadHelper)
  {
    this->QuadHelper->ReleaseGraphicsResources(w);
    this->QuadHelper = nullptr;
  }

  if (this->ColorMapTexture)
  {
    this->ColorMapTexture->ReleaseGraphicsResources(w);
    this->ColorMapTexture = nullptr;
    this->ColorMapTextureValid = false;
  }
}

//------------------------------------------------------------------------------
void vtkF3DPostprocessRenderPass::Render(const vtkRenderState* s)
{
  vtkOpenGLClearErrorMacro();

  this->NumberOfRenderedProps = 0;

  if (!this->ColorTexture)
  {
    vtkWarningMacro(<< " no color texture.");
    return;
  }

  vtkRenderer* r = s->GetRenderer();
  vtkOpenGLRenderWindow* renWin = static_cast<vtkOpenGLRenderWindow*>(r->GetRenderWindow());
  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(r);

  if (!this->QuadHelper)
  {
    std::string fsShader = vtkF3DPostprocessFS;

    if (this->UseToneMapping)
    {
      vtkShaderProgram::Substitute(
        fsShader, "//#define ENABLE_TONEMAPPING", "#define ENABLE_TONEMAPPING");
    }

    if (!this->UserShader.empty())
    {
      vtkShaderProgram::Substitute(fsShader, "//VTK::User::Dec", this->UserShader);
    }
    else
    {
      constexpr std::string_view defaultFunction =
        "vec4 pixel(vec2 uv) { return getLinearColor(uv); }";
      vtkShaderProgram::Substitute(fsShader, "//VTK::User::Dec", defaultFunction.data());
    }

    if (this->ResolutionScale > 1.0)
    {
      vtkShaderProgram::Substitute(fsShader, "//#define ENABLE_SSAA", "#define ENABLE_SSAA");
    }

    if (!this->ColorInputIsLinear)
    {
      vtkShaderProgram::Substitute(
        fsShader, "//VTK::ColorSpace::Impl", "  color.rgb = pow(color.rgb, vec3(2.2));\n");
    }

    this->QuadHelper = std::make_shared<vtkOpenGLQuadHelper>(renWin,
      vtkOpenGLRenderUtilities::GetFullScreenQuadVertexShader().c_str(), fsShader.c_str(), "");
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->QuadHelper->Program);
  }

  if (!this->QuadHelper->Program || !this->QuadHelper->Program->GetCompiled())
  {
    vtkErrorMacro("Couldn't build the full screen shader program.");
    return;
  }

  this->ColorTexture->Activate();
  this->QuadHelper->Program->SetUniformi("texColor", this->ColorTexture->GetTextureUnit());

  if (this->DepthTexture)
  {
    this->DepthTexture->Activate();
    this->QuadHelper->Program->SetUniformi("texDepth", this->DepthTexture->GetTextureUnit());
  }

  if (this->NormalTexture)
  {
    this->NormalTexture->Activate();
    this->QuadHelper->Program->SetUniformi("texNormal", this->NormalTexture->GetTextureUnit());
  }

  this->UpdateColorMapTexture(renWin);
  this->ColorMapTexture->Activate();
  this->QuadHelper->Program->SetUniformi("texColorMap", this->ColorMapTexture->GetTextureUnit());

  this->QuadHelper->Program->SetUniform2i("resolution", renWin->GetSize());
  this->QuadHelper->Program->SetUniformf("ssaaScale", static_cast<float>(this->ResolutionScale));

  if (!ren)
  {
    this->QuadHelper->Program->SetUniformf("time", 0);
  }
  else
  {
    this->QuadHelper->Program->SetUniformf("time", ren->GetTotalTime());
  }

  double bgColor[3];
  r->GetBackground(bgColor);

  int* size = renWin->GetSize();

  vtkF3DRenderPassUtils::SetRenderOptions(vtkOpenGLRenderer::SafeDownCast(r),
    { .BlendMode = vtkF3DRenderPassUtils::BlendStraight,
      .ClearColor = std::array<float, 4>{ static_cast<float>(bgColor[0]),
        static_cast<float>(bgColor[1]), static_cast<float>(bgColor[2]), 0.0f },
      .Width = size[0],
      .Height = size[1] });

  this->QuadHelper->Render();

  this->ColorTexture->Deactivate();
  this->ColorMapTexture->Deactivate();
  if (this->DepthTexture)
  {
    this->DepthTexture->Deactivate();
  }
  if (this->NormalTexture)
  {
    this->NormalTexture->Deactivate();
  }

  vtkOpenGLCheckErrorMacro("failed after Render");
}
