#include "vtkF3DDisplayDepthRenderPass.h"

#include "vtkObjectFactory.h"
#include "vtkOpenGLError.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkOpenGLQuadHelper.h"
#include "vtkOpenGLRenderUtilities.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLShaderCache.h"
#include "vtkOpenGLState.h"
#include "vtkRenderState.h"
#include "vtkRenderer.h"
#include "vtkShaderProgram.h"
#include "vtkTextureObject.h"
#include <vtkOpenGLRenderer.h>

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

  // create framebuffer and textures
  int pos[2];
  int size[2];
  renderer->GetTiledSizeAndOrigin(&size[0], &size[1], &pos[0], &pos[1]);

  if (this->DepthTexture == nullptr)
  {
    this->DepthTexture = vtkSmartPointer<vtkTextureObject>::New();
    this->DepthTexture->SetContext(renWin);
    this->DepthTexture->SetFormat(GL_RGBA);
    this->DepthTexture->SetInternalFormat(GL_RGBA16F);
    this->DepthTexture->SetDataType(GL_HALF_FLOAT);
    this->DepthTexture->SetMinificationFilter(vtkTextureObject::Linear);
    this->DepthTexture->SetMagnificationFilter(vtkTextureObject::Linear);
    this->DepthTexture->SetWrapS(vtkTextureObject::ClampToEdge);
    this->DepthTexture->SetWrapT(vtkTextureObject::ClampToEdge);
    this->DepthTexture->Allocate2D(size[0], size[1], 4, VTK_FLOAT);
  }
  this->DepthTexture->Resize(size[0], size[1]);

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

    if (ColorMapDirty && this->ColorMap)
    {
      int tableSize = 256;
      this->ColorMap->SetNumberOfValues(tableSize);
      this->ColorMap->Build();
      double range[2];
      this->ColorMap->GetRange(range);

      std::vector<float> table(tableSize * 3);
      this->ColorMap->GetTable(range[0], range[1], tableSize, table.data());

      this->ColorMapTexture->Create1DFromRaw(tableSize, 3, VTK_FLOAT, table.data());
      ColorMapDirty = false;
    }
  }

  if (this->FrameBufferObject == nullptr)
  {
    this->FrameBufferObject = vtkSmartPointer<vtkOpenGLFramebufferObject>::New();
    this->FrameBufferObject->SetContext(renWin);
  }

  this->PreRender(state);
  renWin->GetState()->PushFramebufferBindings();
  this->RenderDelegate(
    state, size[0], size[1], size[0], size[1], this->FrameBufferObject, this->DepthTexture);
  renWin->GetState()->PopFramebufferBindings();
  this->PostRender(state);

  if (!this->QuadHelper)
  {
    std::string depthDisplayingFS =
      vtkOpenGLRenderUtilities::GetFullScreenQuadFragmentShaderTemplate();
    vtkShaderProgram::Substitute(depthDisplayingFS, "//VTK::FSQ::Decl",
      "uniform sampler2D depthTexture;\n"
      "uniform bool useColorMap;\n"
      "uniform sampler1D colorMapTexture;\n"
      "//VTK::FSQ::Decl");

    vtkShaderProgram::Substitute(depthDisplayingFS, "//VTK::FSQ::Impl",
      "float depth = texture2D(depthTexture, texCoord).r;\n"
      "if (useColorMap)\n"
      "{\n"
      "  vec4 colorMapped = texture1D(colorMapTexture, depth);\n"
      "  gl_FragData[0] = vec4(colorMapped.rgb, 1.0);\n"
      "  return;\n"
      "}\n"
      "else {"
      "if (depth <= 1e-6) depth = 1.0;\n"
      "}"
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

  this->DepthTexture->Activate();
  this->ColorMapTexture->Activate();

  this->QuadHelper->Program->SetUniformi("depthTexture", this->DepthTexture->GetTextureUnit());
  this->QuadHelper->Program->SetUniformi("useColorMap", this->ColorMap != nullptr ? 1 : 0);
  this->QuadHelper->Program->SetUniformi(
    "colorMapTexture", this->ColorMapTexture->GetTextureUnit());

  ostate->vtkglDisable(GL_BLEND);
  ostate->vtkglDisable(GL_DEPTH_TEST);
  ostate->vtkglViewport(pos[0], pos[1], size[0], size[1]);
  ostate->vtkglScissor(pos[0], pos[1], size[0], size[1]);

  this->QuadHelper->Render();

  this->DepthTexture->Deactivate();
  this->ColorMapTexture->Deactivate();
  vtkOpenGLCheckErrorMacro("failed after Render");
}

void vtkF3DDisplayDepthRenderPass::ReleaseGraphicsResources(vtkWindow* window)
{
  this->Superclass::ReleaseGraphicsResources(window);

  if (this->FrameBufferObject)
  {
    this->FrameBufferObject->ReleaseGraphicsResources(window);
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

bool vtkF3DDisplayDepthRenderPass::PreReplaceShaderValues(std::string& vertexShader,
  std::string& geometryShader, std::string& fragmentShader, vtkAbstractMapper* mapper,
  vtkProp* prop)
{
  vtkShaderProgram::Substitute(
    fragmentShader, "//VTK::Light::Impl", "  gl_FragData[0] = vec4(vec3(gl_FragCoord.z), 1.0);\n");
  return true;
}

void vtkF3DDisplayDepthRenderPass::SetColorMap(vtkDiscretizableColorTransferFunction* colorMap)
{
  this->ColorMap = colorMap;
  ColorMapDirty = true;
}