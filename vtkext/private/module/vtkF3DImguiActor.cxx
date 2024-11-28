#include "vtkF3DImguiActor.h"

#include "vtkF3DImguiFS.h"
#include "vtkF3DImguiVS.h"
#include "F3DFontBuffer.h"

#include <vtkObjectFactory.h>
#include <vtkOpenGLBufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkOpenGLVertexArrayObject.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkShader.h>
#include <vtkShaderProgram.h>
#include <vtkTextureObject.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtk_glad.h>
#else
#include <vtk_glew.h>
#endif

#include <imgui.h>

struct vtkF3DImguiActor::Internals
{
  void Initialize(vtkOpenGLRenderWindow* renWin)
  {
    if (this->FontTexture == nullptr)
    {
      // Build texture atlas
      ImGuiIO& io = ImGui::GetIO();
      unsigned char* pixels;
      int width, height;
      io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

      this->FontTexture = vtkSmartPointer<vtkTextureObject>::New();
      this->FontTexture->SetContext(renWin);
      this->FontTexture->Create2DFromRaw(width, height, 4, VTK_UNSIGNED_CHAR, pixels);

      // Store our identifier
      io.Fonts->SetTexID((ImTextureID)this->FontTexture.Get());

      // Create VBO
      this->VertexBuffer = vtkSmartPointer<vtkOpenGLBufferObject>::New();

      // https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10589
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20231016)
      this->VertexBuffer->SetUsage(vtkOpenGLBufferObject::StreamDraw);
#endif
      this->VertexBuffer->GenerateBuffer(vtkOpenGLBufferObject::ArrayBuffer);

      // Create IBO
      this->IndexBuffer = vtkSmartPointer<vtkOpenGLBufferObject>::New();

      // https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10589
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20231016)
      this->IndexBuffer->SetUsage(vtkOpenGLBufferObject::StreamDraw);
#endif
      this->IndexBuffer->GenerateBuffer(vtkOpenGLBufferObject::ElementArrayBuffer);

      // Create shader program
      std::string emptyGeom; // no geometry shader
      this->Program =
        renWin->GetShaderCache()->ReadyShaderProgram(vtkF3DImguiVS, vtkF3DImguiFS, emptyGeom.c_str());

      // Create VAO
      this->VertexArray = vtkSmartPointer<vtkOpenGLVertexArrayObject>::New();
      this->VertexArray->Bind();
      this->VertexArray->AddAttributeArray(
        this->Program, this->VertexBuffer, "Position", 0, sizeof(ImDrawVert), VTK_FLOAT, 2, false);
      this->VertexArray->AddAttributeArray(
        this->Program, this->VertexBuffer, "UV", 8, sizeof(ImDrawVert), VTK_FLOAT, 2, false);
      this->VertexArray->AddAttributeArray(this->Program, this->VertexBuffer, "Color", 16,
        sizeof(ImDrawVert), VTK_UNSIGNED_CHAR, 4, true);
    }
  }

  void Release(vtkOpenGLRenderWindow* renWin)
  {
    if (ImGui::GetCurrentContext() != nullptr)
    {
      ImGuiIO& io = ImGui::GetIO();

      if (this->FontTexture)
      {
        io.Fonts->SetTexID(0);
        this->FontTexture->ReleaseGraphicsResources(renWin);
        this->FontTexture = nullptr;
      }

      if (this->VertexBuffer)
      {
        this->VertexBuffer = nullptr;
      }

      if (this->IndexBuffer)
      {
        this->IndexBuffer = nullptr;
      }

      if (this->Program)
      {
        this->Program->ReleaseGraphicsResources(renWin);
        this->Program = nullptr;
      }

      io.Fonts->Clear();

      io.BackendPlatformName = io.BackendRendererName = nullptr;
      ImGui::DestroyContext();
    }
  }

  void RenderDrawData(vtkOpenGLRenderWindow* renWin, ImDrawData* drawData)
  {
    vtkOpenGLState* state = renWin->GetState();

    vtkOpenGLState::ScopedglScissor save_scissorbox(state);
    vtkOpenGLState::ScopedglBlendFuncSeparate save_blendfunc(state);
    vtkOpenGLState::ScopedglEnableDisable save_blend(state, GL_BLEND);
    vtkOpenGLState::ScopedglEnableDisable save_cull(state, GL_CULL_FACE);
    vtkOpenGLState::ScopedglEnableDisable save_depth(state, GL_DEPTH_TEST);
    vtkOpenGLState::ScopedglEnableDisable save_stencil(state, GL_STENCIL_TEST);
    vtkOpenGLState::ScopedglEnableDisable save_scissor(state, GL_SCISSOR_TEST);

    // Change require OpenGL state for proper rendering
    state->vtkglEnable(GL_BLEND);
    state->vtkglBlendEquation(GL_FUNC_ADD);
    state->vtkglBlendFuncSeparate(
      GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    state->vtkglDisable(GL_CULL_FACE);
    state->vtkglDisable(GL_DEPTH_TEST);
    state->vtkglDisable(GL_STENCIL_TEST);
    state->vtkglEnable(GL_SCISSOR_TEST);

    renWin->GetShaderCache()->ReadyShaderProgram(this->Program);

    // Set scale/shift (Y is inverted in OpenGL)
    float scale[2], shift[2];
    scale[0] = 2.f / drawData->DisplaySize.x;
    scale[1] = -2.f / drawData->DisplaySize.y;
    shift[0] = -(2.f * drawData->DisplayPos.x + drawData->DisplaySize.x) / drawData->DisplaySize.x;
    shift[1] = (2.f * drawData->DisplayPos.y + drawData->DisplaySize.y) / drawData->DisplaySize.y;

    this->FontTexture->Activate();

    this->Program->SetUniform2f("Scale", scale);
    this->Program->SetUniform2f("Shift", shift);
    this->Program->SetUniformi("Texture", this->FontTexture->GetTextureUnit());
    
    // Render the UI
    this->VertexArray->Bind();
    this->VertexBuffer->Bind();
    this->IndexBuffer->Bind();

    ImVec2 clipOff = drawData->DisplayPos;
    ImVec2 clipScale = drawData->FramebufferScale;

    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
      const ImDrawList* cmdList = drawData->CmdLists[n];

      this->VertexBuffer->Upload(
        cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size, vtkOpenGLBufferObject::ArrayBuffer);
      this->IndexBuffer->Upload(cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size,
        vtkOpenGLBufferObject::ElementArrayBuffer);

      for (int iCmd = 0; iCmd < cmdList->CmdBuffer.Size; iCmd++)
      {
        const ImDrawCmd* cmd = &cmdList->CmdBuffer[iCmd];

        // Project scissor/clipping rectangles into framebuffer space
        ImVec2 clipMin(
          (cmd->ClipRect.x - clipOff.x) * clipScale.x, (cmd->ClipRect.y - clipOff.y) * clipScale.y);
        ImVec2 clipMax(
          (cmd->ClipRect.z - clipOff.x) * clipScale.x, (cmd->ClipRect.w - clipOff.y) * clipScale.y);
        if (clipMax.x > clipMin.x && clipMax.y > clipMin.y)
        {
          // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
          float fbHeight = drawData->DisplaySize.y * drawData->FramebufferScale.y;
          state->vtkglScissor(static_cast<GLint>(clipMin.x), static_cast<GLint>(fbHeight - clipMax.y),
            static_cast<GLsizei>(clipMax.x - clipMin.x), static_cast<GLsizei>(clipMax.y - clipMin.y));

          glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cmd->ElemCount),
            sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
            reinterpret_cast<void*>(cmd->IdxOffset * sizeof(ImDrawIdx)));
        }
      }
    }

    this->VertexArray->Release();
    this->VertexBuffer->Release();
    this->IndexBuffer->Release();

    this->FontTexture->Deactivate();
  }

  vtkSmartPointer<vtkTextureObject> FontTexture;
  vtkSmartPointer<vtkOpenGLVertexArrayObject> VertexArray;
  vtkSmartPointer<vtkOpenGLBufferObject> VertexBuffer;
  vtkSmartPointer<vtkOpenGLBufferObject> IndexBuffer;
  vtkSmartPointer<vtkShaderProgram> Program;
};

vtkStandardNewMacro(vtkF3DImguiActor);

//----------------------------------------------------------------------------
vtkF3DImguiActor::vtkF3DImguiActor()
  : Pimpl(new Internals())
{
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::Initialize(vtkOpenGLRenderWindow* renWin)
{
  // release existing context
  this->ReleaseGraphicsResources(renWin);

  ImGuiContext* ctx = ImGui::CreateContext();
  ImGui::SetCurrentContext(ctx);

  ImGuiIO& io = ImGui::GetIO();

  ImFontConfig fontConfig;

  ImFont* font = nullptr;
  if (this->FontFile.empty())
  {
    fontConfig.FontDataOwnedByAtlas = false;
    font = io.Fonts->AddFontFromMemoryTTF((void*)F3DFontBuffer, sizeof(F3DFontBuffer), 18, &fontConfig);
  }
  else
  {
    font = io.Fonts->AddFontFromFileTTF(this->FontFile.c_str(), 18, &fontConfig);
  }
  
  io.Fonts->Build();
  io.FontDefault = font;

  ImGuiStyle* style = &ImGui::GetStyle();
  style->GrabRounding = 4.0f;
  style->WindowRounding = 8.f;
  style->WindowBorderSize = 0.f;
  style->WindowPadding = ImVec2(10, 10);

  // Setup backend name
  io.BackendPlatformName = io.BackendRendererName = "F3D/VTK";
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Pimpl->Release(vtkOpenGLRenderWindow::SafeDownCast(w));
}

//----------------------------------------------------------------------------
vtkF3DImguiActor::~vtkF3DImguiActor() = default;

//----------------------------------------------------------------------------
void vtkF3DImguiActor::RenderFileName()
{
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  constexpr float marginTop = 5.f;
  ImVec2 winSize = ImGui::CalcTextSize(this->FileName.c_str());
  winSize.x += 2.f * ImGui::GetStyle().WindowPadding.x;
  winSize.y += 2.f * ImGui::GetStyle().WindowPadding.y;

  // it's super important to set the size of the window manually
  // otherwise ImGui skip a frame for computing the size resulting in
  // no UI when doing offscreen rendering
  ImGui::SetNextWindowSize(winSize);
  ImGui::SetNextWindowPos(ImVec2(viewport->GetWorkCenter().x - 0.5f * winSize.x, marginTop));
  ImGui::SetNextWindowBgAlpha(0.35f);

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

  ImGui::Begin("FileName", nullptr, flags);
  ImGui::TextUnformatted(this->FileName.c_str());
  ImGui::End();
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::StartFrame(vtkOpenGLRenderWindow* renWin)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    this->Initialize(renWin);
  }

  int* size = renWin->GetSize();

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(static_cast<float>(size[0]), static_cast<float>(size[1]));

  this->Pimpl->Initialize(renWin);

  ImGui::NewFrame();
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::EndFrame(vtkOpenGLRenderWindow* renWin)
{
  ImGui::Render();
  this->Pimpl->RenderDrawData(renWin, ImGui::GetDrawData());
}
