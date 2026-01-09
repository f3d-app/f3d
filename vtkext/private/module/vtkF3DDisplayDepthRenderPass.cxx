#include "vtkF3DDisplayDepthRenderPass.h"

#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLQuadHelper.h>
#include <vtkOpenGLRenderUtilities.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkRenderState.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkTextureObject.h>

vtkStandardNewMacro(vtkF3DDisplayDepthRenderPass);

//------------------------------------------------------------------------------
void vtkF3DDisplayDepthRenderPass::Render(const vtkRenderState* state)
{
  vtkOpenGLClearErrorMacro();
  this->NumberOfRenderedProps = 0;

  vtkRenderer* renderer = state->GetRenderer();
  vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(renderer->GetRenderWindow());
  vtkOpenGLState* ostate = renWin->GetState();

  vtkOpenGLState::ScopedglEnableDisable bsaver(ostate, GL_BLEND);
  vtkOpenGLState::ScopedglEnableDisable dsaver(ostate, GL_DEPTH_TEST);

  assert(this->DelegatePass != nullptr);

  int pos[2];
  int size[2];
  renderer->GetTiledSizeAndOrigin(&size[0], &size[1], &pos[0], &pos[1]);

  InitializeResources(renWin, size[0], size[1]);

  // render to color and depth texture
  renWin->GetState()->PushFramebufferBindings();
  this->RenderDelegate(state, size[0], size[1], size[0], size[1], this->FrameBufferObject,
    this->ColorTexture, this->DepthTexture);
  renWin->GetState()->PopFramebufferBindings();

  // depth displaying pass
  if (!this->QuadHelper)
  {
    std::string depthDisplayingFS =
      vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();
    vtkShaderProgram::Substitute(depthDisplayingFS, "//VTK::FSQ::Decl",
      "uniform sampler2D texDepth;\n"
      "uniform bool useColorMap;\n"
      "uniform sampler1D colorMapTexture;\n"
      "//VTK::FSQ::Decl");

    vtkShaderProgram::Substitute(depthDisplayingFS, "//VTK::FSQ::Impl",
      "float depth = texture(texDepth, texCoord).r;\n"
      "if (useColorMap)\n"
      "{\n"
      "  vec4 colorMapped = texture(colorMapTexture, depth);\n"
      "  gl_FragData[0] = vec4(colorMapped.rgb, 1.0);\n"
      "  return;\n"
      "}\n"
      "gl_FragData[0] = vec4(depth, depth, depth, 1.0);\n"
      "//VTK::FSQ::Impl");
    this->QuadHelper =
      std::make_shared<vtkOpenGLQuadHelper>(renWin, nullptr, depthDisplayingFS.c_str(), nullptr);
    this->QuadHelper->ShaderChangeValue = this->GetMTime();
  }
  else
  {
    renWin->GetShaderCache()->ReadyShaderProgram(this->QuadHelper->Program);
  }

  assert(this->QuadHelper->Program && this->QuadHelper->Program->GetCompiled());

  this->ColorTexture->Activate();
  this->DepthTexture->Activate();
  this->ColorMapTexture->Activate();

  this->QuadHelper->Program->SetUniformi("texDepth", this->DepthTexture->GetTextureUnit());
  this->QuadHelper->Program->SetUniformi("useColorMap", this->ColorMap != nullptr ? 1 : 0);
  this->QuadHelper->Program->SetUniformi(
    "colorMapTexture", this->ColorMapTexture->GetTextureUnit());

  ostate->vtkglEnable(GL_DEPTH_TEST);
  ostate->vtkglDepthFunc(GL_LEQUAL);
  ostate->vtkglClear(GL_DEPTH_BUFFER_BIT);

  this->QuadHelper->Render();

  this->ColorTexture->Deactivate();
  this->DepthTexture->Deactivate();
  this->ColorMapTexture->Deactivate();
  vtkOpenGLCheckErrorMacro("failed after Render");
}

void vtkF3DDisplayDepthRenderPass::InitializeResources(vtkOpenGLRenderWindow* renWin, int w, int h)
{
  if (this->DepthTexture == nullptr)
  {
    this->DepthTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->DepthTexture->SetContext(renWin);
    this->DepthTexture->AllocateDepth(w, h, vtkTextureObject::Float32);
  }
  this->DepthTexture->Resize(w, h);

  if (this->ColorTexture == nullptr)
  {
    this->ColorTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->ColorTexture->SetContext(renWin);
    this->ColorTexture->SetFormat(GL_RGBA);
    this->ColorTexture->SetInternalFormat(GL_RGBA8);
    this->ColorTexture->SetDataType(GL_UNSIGNED_BYTE);
    this->ColorTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->ColorTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->ColorTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->ColorTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->ColorTexture->Allocate2D(w, h, 4, VTK_UNSIGNED_CHAR);
  }
  this->ColorTexture->Resize(w, h);

  if (this->ColorMapTexture == nullptr)
  {
    this->ColorMapTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->ColorMapTexture->SetContext(renWin);
    this->ColorMapTexture->Allocate1D(256, 3, VTK_FLOAT);
    this->ColorMapTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->ColorMapTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->ColorMapTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->ColorMapTexture->SetMagnificationFilter(vtkTextureObject::Linear);

    this->ColorMapTexture->SetFormat(GL_RGB);
    this->ColorMapTexture->SetInternalFormat(GL_RGB16F);
    this->ColorMapTexture->SetDataType(GL_FLOAT);

    if (this->ColorMap && this->ColorMapBuildTime.GetMTime() < this->ColorMap->GetMTime())
    {
      constexpr int tableSize = 256;
      this->ColorMap->SetNumberOfValues(tableSize);
      this->ColorMap->Build();
      double range[2];
      this->ColorMap->GetRange(range);

      std::vector<float> table(tableSize * 3);
      this->ColorMap->GetTable(range[0], range[1], tableSize, table.data());

      this->ColorMapTexture->Create1DFromRaw(tableSize, 3, VTK_FLOAT, table.data());
      this->ColorMapBuildTime.Modified();
    }
  }

  if (this->FrameBufferObject == nullptr)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->FrameBufferObject->SetContext(renWin);
  }
}

void vtkF3DDisplayDepthRenderPass::ReleaseGraphicsResources(vtkWindow* window)
{
  this->Superclass::ReleaseGraphicsResources(window);

  if (this->FrameBufferObject)
  {
    this->FrameBufferObject->ReleaseGraphicsResources(window);
  }
  if (this->ColorTexture)
  {
    this->ColorTexture->ReleaseGraphicsResources(window);
  }
  if (this->DepthTexture)
  {
    this->DepthTexture->ReleaseGraphicsResources(window);
  }
  if (this->ColorMapTexture)
  {
    this->ColorMapTexture->ReleaseGraphicsResources(window);
  }
}

void vtkF3DDisplayDepthRenderPass::SetColorMap(vtkDiscretizableColorTransferFunction* colorMap)
{
  this->ColorMap = colorMap;
}
