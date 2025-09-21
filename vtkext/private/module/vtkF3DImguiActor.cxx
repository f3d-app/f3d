#include "vtkF3DImguiActor.h"

#include "F3DDefaultLogo.h"
#include "F3DFontBuffer.h"
#include "F3DImguiStyle.h"
#include "vtkF3DImguiConsole.h"
#include "vtkF3DImguiFS.h"
#include "vtkF3DImguiVS.h"

#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLBufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLState.h>
#include <vtkOpenGLVertexArrayObject.h>
#include <vtkPNGReader.h>
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
#include <numeric>
#include <optional>
#include <sstream>
#include <string>

namespace
{
constexpr float LOGO_DISPLAY_WIDTH = 256.f;
constexpr float LOGO_DISPLAY_HEIGHT = 256.f;
constexpr float DROPZONE_LOGO_TEXT_PADDING = 20.f;
constexpr float DROPZONE_MARGIN = 0.5f;
constexpr float DROPZONE_PADDING_X = 5.0f;
constexpr float DROPZONE_PADDING_Y = 2.0f;

static std::vector<std::string> SplitBindings(const std::string& s, char delim)
{
  std::vector<std::string> result;
  std::stringstream ss(s);
  std::string item;

  while (std::getline(ss, item, delim))
  {
    result.push_back(item);
  }

  return result;
}
}

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

      // Load embedded PNG icon into texture
      vtkNew<vtkPNGReader> iconReader;
      iconReader->SetMemoryBuffer(F3DDefaultLogo);
      iconReader->SetMemoryBufferLength(sizeof(F3DDefaultLogo));
      iconReader->Update();

      vtkImageData* imageData = iconReader->GetOutput();
      int* dims = imageData->GetDimensions();

      unsigned char* logoPixels = static_cast<unsigned char*>(imageData->GetScalarPointer());
      if (logoPixels)
      {
        this->LogoTexture = vtkSmartPointer<vtkTextureObject>::New();
        this->LogoTexture->SetContext(renWin);
        this->LogoTexture->Create2DFromRaw(dims[0], dims[1], 4, VTK_UNSIGNED_CHAR, logoPixels);
      }

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
      this->Program = renWin->GetShaderCache()->ReadyShaderProgram(
        vtkF3DImguiVS, vtkF3DImguiFS, emptyGeom.c_str());

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
      if (this->LogoTexture)
      {
        this->LogoTexture->ReleaseGraphicsResources(renWin);
        this->LogoTexture = nullptr;
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

        // Activate texture and set uniforms per draw command:
        vtkTextureObject* texObj = reinterpret_cast<vtkTextureObject*>(cmd->GetTexID());
        texObj->Activate();
        this->Program->SetUniform2f("Scale", scale);
        this->Program->SetUniform2f("Shift", shift);
        this->Program->SetUniformi("Texture", texObj->GetTextureUnit());

        // Project scissor/clipping rectangles into framebuffer space
        ImVec2 clipMin(
          (cmd->ClipRect.x - clipOff.x) * clipScale.x, (cmd->ClipRect.y - clipOff.y) * clipScale.y);
        ImVec2 clipMax(
          (cmd->ClipRect.z - clipOff.x) * clipScale.x, (cmd->ClipRect.w - clipOff.y) * clipScale.y);
        if (clipMax.x > clipMin.x && clipMax.y > clipMin.y)
        {
          // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
          float fbHeight = drawData->DisplaySize.y * drawData->FramebufferScale.y;
          state->vtkglScissor(static_cast<GLint>(clipMin.x),
            static_cast<GLint>(fbHeight - clipMax.y), static_cast<GLsizei>(clipMax.x - clipMin.x),
            static_cast<GLsizei>(clipMax.y - clipMin.y));

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
    this->LogoTexture->Deactivate();
  }

  vtkSmartPointer<vtkTextureObject> FontTexture;
  vtkSmartPointer<vtkOpenGLVertexArrayObject> VertexArray;
  vtkSmartPointer<vtkOpenGLBufferObject> VertexBuffer;
  vtkSmartPointer<vtkOpenGLBufferObject> IndexBuffer;
  vtkSmartPointer<vtkShaderProgram> Program;
  vtkSmartPointer<vtkTextureObject> LogoTexture;
};

namespace
{
void SetupNextWindow(std::optional<ImVec2> position, std::optional<ImVec2> size)
{
  if (size.has_value())
  {
    // it's super important to set the size of the window manually
    // otherwise ImGui skip a frame for computing the size resulting in
    // no UI when doing offscreen rendering
    ImGui::SetNextWindowSize(size.value());
  }

  if (position.has_value())
  {
    ImGui::SetNextWindowPos(position.value());
  }
}
}

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
  io.IniFilename = nullptr;
  io.LogFilename = nullptr;

  ImFontConfig fontConfig;

  ImVector<ImWchar> ranges;
  ImFontGlyphRangesBuilder builder;
  builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
  builder.AddChar(0x2264); // Less-Than or Equal To
  builder.BuildRanges(&ranges);

  ImFont* font = nullptr;
  if (this->FontFile.empty())
  {
    // ImGui API is not very helpful with this
    fontConfig.FontDataOwnedByAtlas = false;
    font = io.Fonts->AddFontFromMemoryTTF(
      const_cast<void*>(reinterpret_cast<const void*>(F3DFontBuffer)), sizeof(F3DFontBuffer), 18,
      &fontConfig, ranges.Data);
  }
  else
  {
    font = io.Fonts->AddFontFromFileTTF(this->FontFile.c_str(), 18, &fontConfig, ranges.Data);
  }

  io.Fonts->Build();
  io.FontDefault = font;
  io.FontGlobalScale = this->FontScale;

  ImVec4 colTransparent = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); // #000000

  ImGuiStyle* style = &ImGui::GetStyle();
  style->AntiAliasedLines = false;
  style->GrabRounding = 4.0f;
  style->WindowPadding = ImVec2(10, 10);
  style->WindowRounding = 8.f;
  style->WindowBorderSize = 0.f;
  style->FrameBorderSize = 0.f;
  style->FramePadding = ImVec2(4, 2);
  style->FrameRounding = 2.f;
  style->Colors[ImGuiCol_Text] = F3DImguiStyle::GetTextColor();
  style->Colors[ImGuiCol_WindowBg] = F3DImguiStyle::GetBackgroundColor();
  style->Colors[ImGuiCol_FrameBg] = colTransparent;
  style->Colors[ImGuiCol_FrameBgActive] = colTransparent;
  style->Colors[ImGuiCol_ScrollbarBg] = colTransparent;
  style->Colors[ImGuiCol_ScrollbarGrab] = F3DImguiStyle::GetMidColor();
  style->Colors[ImGuiCol_ScrollbarGrabHovered] = F3DImguiStyle::GetHighlightColor();
  style->Colors[ImGuiCol_ScrollbarGrabActive] = F3DImguiStyle::GetHighlightColor();
  style->Colors[ImGuiCol_TextSelectedBg] = F3DImguiStyle::GetHighlightColor();

  // Setup backend name
  io.BackendPlatformName = io.BackendRendererName = "F3D/VTK";
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);

  this->Pimpl->Release(vtkOpenGLRenderWindow::SafeDownCast(w));
}

//----------------------------------------------------------------------------
vtkF3DImguiActor::~vtkF3DImguiActor() = default;

//----------------------------------------------------------------------------
void vtkF3DImguiActor::RenderDropZone()
{
  if (this->DropZoneVisible)
  {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    if (viewport->WorkSize.x < 10 || viewport->WorkSize.y < 10)
    {
      return;
    }

    constexpr ImVec4 colorImv = F3DImguiStyle::GetTextColor();
    constexpr ImU32 color =
      IM_COL32(colorImv.x * 255, colorImv.y * 255, colorImv.z * 255, colorImv.w * 255);

    const int dropzonePad =
      static_cast<int>(std::min(viewport->WorkSize.x, viewport->WorkSize.y) * 0.1);
    const int dropZoneW = viewport->WorkSize.x - dropzonePad * 2;
    const int dropZoneH = viewport->WorkSize.y - dropzonePad * 2;

    constexpr float tickThickness = 3.0f;
    constexpr float tickLength = 10.0f;
    const int halfTickThickness = static_cast<int>(std::ceil(tickThickness / 2.f));

    const int tickNumberW = static_cast<int>(std::ceil(dropZoneW / (tickLength * 2.0f)));
    const int tickNumberH = static_cast<int>(std::ceil(dropZoneH / (tickLength * 2.0f)));

    const double tickSpaceW =
      static_cast<double>(dropZoneW - tickNumberW * tickLength + 1) / (tickNumberW - 1);
    const double tickSpaceH =
      static_cast<double>(dropZoneH - tickNumberH * tickLength + 1) / (tickNumberH - 1);

    ::SetupNextWindow(ImVec2(0, 0), viewport->WorkSize);
    ImGui::SetNextWindowBgAlpha(0.f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMouseInputs;

    ImGui::Begin("DropZoneText", nullptr, flags);
    /* Use background draw list to prevent "ignoring" NoBringToFrontOnFocus */
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Logo rendering
    if (this->DropZoneLogoVisible && this->Pimpl->LogoTexture)
    {
      float logoDisplayWidth = ::LOGO_DISPLAY_WIDTH;
      float logoDisplayHeight = ::LOGO_DISPLAY_HEIGHT;
      ImVec2 center = viewport->GetWorkCenter();
      ImVec2 logoPos(center.x - logoDisplayWidth * ::DROPZONE_MARGIN,
        center.y - logoDisplayHeight * ::DROPZONE_MARGIN);

      // VTK texture pointer to ImTextureID cast (void*)
      ImTextureID texID = reinterpret_cast<ImTextureID>(this->Pimpl->LogoTexture.Get());

      drawList->AddImage(texID, logoPos,
        ImVec2(logoPos.x + logoDisplayWidth, logoPos.y + logoDisplayHeight), ImVec2(0, 1),
        ImVec2(1, 0));
    }

    const ImVec2 p0(dropzonePad, dropzonePad);
    const ImVec2 p1(dropzonePad + dropZoneW, dropzonePad + dropZoneH);

    // Border lines
    for (float x = p0.x - 1; x < p1.x; x += tickLength + tickSpaceW)
    {
      const float y0 = p0.y + halfTickThickness;
      const float x1 = std::min(p1.x, x + tickLength);
      drawList->AddLine(ImVec2(x, y0), ImVec2(x1, y0), color, tickThickness);
      drawList->AddLine(ImVec2(x, p1.y), ImVec2(x1, p1.y), color, tickThickness);
    }

    // Draw left and right line
    for (float y = p0.y; y < p1.y; y += tickLength + tickSpaceH)
    {
      const float x1 = p1.x - halfTickThickness;
      const float y1 = std::min(p1.y, y + tickLength);
      drawList->AddLine(ImVec2(p0.x, y), ImVec2(p0.x, y1), color, tickThickness);
      drawList->AddLine(ImVec2(x1, y), ImVec2(x1, y1), color, tickThickness);
    }

    ImGui::End();

    // If DropText is provided, render and skip binds
    if (!this->DropText.empty())
    {
      ImVec2 textSize = ImGui::CalcTextSize(this->DropText.c_str());
      ImVec2 textPos(viewport->GetWorkCenter().x - textSize.x * ::DROPZONE_MARGIN,
        viewport->GetWorkCenter().y - ::DROPZONE_MARGIN * textSize.y + ::LOGO_DISPLAY_HEIGHT / 2 +
          ::DROPZONE_LOGO_TEXT_PADDING);
      drawList->AddText(textPos, ImColor(F3DImguiStyle::GetTextColor()), this->DropText.c_str());
      return;
    }

    float maxDescTextWidth = 0.0f;
    float maxBindingsTextWidth = 0.0f;
    const float spacingX = ImGui::GetStyle().ItemSpacing.x;
    const float plusWidth = ImGui::CalcTextSize("+").x;

    // Compute widths
    for (const auto& pair : this->DropBinds)
    {
      const auto& desc = pair.first;
      const auto& bind = pair.second;
      float totalBindingsWidth = 0.0f;

      ImVec2 descSize = ImGui::CalcTextSize(desc.c_str());
      maxDescTextWidth = std::max(maxDescTextWidth, descSize.x);

      auto keys = ::SplitBindings(bind, '+');

      totalBindingsWidth += std::accumulate(keys.begin(), keys.end(),
        0.0f, // use float init since CalcTextSize returns float
        [](float sum, const std::string& key)
        {
          return sum + ImGui::CalcTextSize(key.c_str()).x +
            ::DROPZONE_MARGIN * ::DROPZONE_LOGO_TEXT_PADDING;
        });

      if (keys.size() > 1)
      {
        totalBindingsWidth += (keys.size() - 1) * (spacingX + plusWidth + spacingX);
      }

      maxBindingsTextWidth = std::max(maxBindingsTextWidth, totalBindingsWidth);
    }

    const ImColor descTextColor = F3DImguiStyle::GetTextColor();
    const ImColor bindingRectColor = F3DImguiStyle::GetMidColor();
    const ImColor bindingTextColor = F3DImguiStyle::GetTextColor();

    float tableWidth = maxDescTextWidth + maxBindingsTextWidth + ::DROPZONE_LOGO_TEXT_PADDING +
      ImGui::GetStyle().ItemSpacing.x;

    // Position table below logo if needed
    ImVec2 startPos;
    if (this->DropZoneLogoVisible && this->Pimpl->LogoTexture)
    {
      startPos = ImVec2(viewport->GetWorkCenter().x - tableWidth * ::DROPZONE_MARGIN,
        viewport->GetWorkCenter().y + ::LOGO_DISPLAY_HEIGHT / 2 + ::DROPZONE_MARGIN);
    }
    else
    {
      startPos = ImVec2(
        viewport->GetWorkCenter().x - tableWidth * ::DROPZONE_MARGIN, viewport->GetWorkCenter().y);
    }

    ImVec2 cursor = startPos;

    for (const auto& pair : this->DropBinds)
    {
      const auto& desc = pair.first;
      const auto& bind = pair.second;

      drawList->AddText(cursor, descTextColor, desc.c_str());
      float rowHeight =
        ImGui::GetTextLineHeightWithSpacing() + ::DROPZONE_MARGIN * ::DROPZONE_LOGO_TEXT_PADDING;

      float xBindings = cursor.x + maxDescTextWidth + ::DROPZONE_LOGO_TEXT_PADDING;
      ImVec2 bindingPos(xBindings, cursor.y);

      auto keys = ::SplitBindings(bind, '+');
      for (size_t k = 0; k < keys.size(); ++k)
      {
        const std::string& key = keys[k];
        ImVec2 textSize = ImGui::CalcTextSize(key.c_str());
        ImVec2 padding(::DROPZONE_PADDING_X, ::DROPZONE_PADDING_Y);

        ImVec2 rectMin = ImVec2(bindingPos.x, bindingPos.y);
        ImVec2 rectMax =
          ImVec2(rectMin.x + textSize.x + padding.x * 2, rectMin.y + textSize.y + padding.y * 2);

        drawList->AddRectFilled(rectMin, rectMax, bindingRectColor, 4.0f);
        drawList->AddText(
          ImVec2(rectMin.x + padding.x, rectMin.y + padding.y), bindingTextColor, key.c_str());

        bindingPos.x = rectMax.x + ImGui::GetStyle().ItemSpacing.x;

        if (k < keys.size() - 1)
        {
          drawList->AddText(bindingPos, descTextColor, "+");
          bindingPos.x += plusWidth + ImGui::GetStyle().ItemSpacing.x;
        }
      }
      cursor.y += rowHeight;
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::RenderFileName()
{
  if (!this->FileName.empty())
  {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    constexpr float margin = F3DImguiStyle::GetDefaultMargin();
    ImVec2 winSize = ImGui::CalcTextSize(this->FileName.c_str());
    winSize.x += 2.f * ImGui::GetStyle().WindowPadding.x;
    winSize.y += 2.f * ImGui::GetStyle().WindowPadding.y;

    ::SetupNextWindow(ImVec2(viewport->GetWorkCenter().x - 0.5f * winSize.x, margin), winSize);
    ImGui::SetNextWindowBgAlpha(this->BackdropOpacity);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    ImGui::Begin("FileName", nullptr, flags);
    ImGui::TextUnformatted(this->FileName.c_str());
    ImGui::End();
  }
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::RenderMetaData()
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  constexpr float margin = F3DImguiStyle::GetDefaultMargin();

  ImVec2 winSize = ImGui::CalcTextSize(this->MetaData.c_str());
  winSize.x += 2.f * ImGui::GetStyle().WindowPadding.x;
  winSize.y += 2.f * ImGui::GetStyle().WindowPadding.y;

  ::SetupNextWindow(ImVec2(viewport->WorkSize.x - winSize.x - margin,
                      viewport->GetWorkCenter().y - 0.5f * winSize.y),
    winSize);
  ImGui::SetNextWindowBgAlpha(this->BackdropOpacity);

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

  ImGui::Begin("MetaData", nullptr, flags);
  ImGui::TextUnformatted(this->MetaData.c_str());
  ImGui::End();
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::RenderCheatSheet()
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  constexpr float margin = F3DImguiStyle::GetDefaultMargin();
  constexpr float padding = 16.f;

  float textHeight = 0.f;
  float winWidth = 0.f;

  // Use to create all rect with same size
  float maxBindingTextWidth = 0.f;
  float maxDescTextWidth = 0.f;
  float maxValueTextWidth = 0.f;

  for (const auto& [group, content] : this->CheatSheet)
  {
    textHeight +=
      ImGui::GetTextLineHeightWithSpacing() + 2 * ImGui::GetStyle().SeparatorTextPadding.y;
    for (const auto& [bind, desc, val, type] : content)
    {
      textHeight += ImGui::GetTextLineHeightWithSpacing();

      ImVec2 bindingLineSize = ImGui::CalcTextSize(bind.c_str());
      maxBindingTextWidth = std::max(maxBindingTextWidth, bindingLineSize.x);

      ImVec2 descriptionLineSize = ImGui::CalcTextSize(desc.c_str());
      maxDescTextWidth = std::max(maxDescTextWidth, descriptionLineSize.x);

      std::string cyclingValue = "< " + val + " >";
      ImVec2 valueLineSize = ImGui::CalcTextSize(cyclingValue.c_str());
      maxValueTextWidth = std::max(maxValueTextWidth, valueLineSize.x);

      winWidth = maxBindingTextWidth + maxDescTextWidth + maxValueTextWidth;
    }
  }

  winWidth += ImGui::GetStyle().ScrollbarSize + 4.f * padding;
  textHeight += 2.f * ImGui::GetStyle().WindowPadding.y;

  const float winTop = std::max(margin, (viewport->WorkSize.y - textHeight) * 0.5f);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));

  ::SetupNextWindow(ImVec2(margin, winTop),
    ImVec2(winWidth, std::min(viewport->WorkSize.y - (2 * margin), textHeight)));
  ImGui::SetNextWindowBgAlpha(this->BackdropOpacity);

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBringToFrontOnFocus;

  ImGui::Begin("CheatSheet", nullptr, flags);

  for (const auto& [group, list] : this->CheatSheet)
  {
    ImGui::SeparatorText(group.c_str());
    ImGui::BeginTable("BindingsTable", 3);
    ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthFixed, maxDescTextWidth);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, maxValueTextWidth);
    ImGui::TableSetupColumn("Bindings", ImGuiTableColumnFlags_WidthStretch, maxBindingTextWidth);
    for (const auto& [bind, desc, val, type] : list)
    {
      ImVec4 bindingTextColor, bindingRectColor, descTextColor, valueTextColor;

      if (type == CheatSheetBindingType::TOGGLE && val == "ON")
      {
        bindingTextColor = F3DImguiStyle::GetBackgroundColor();
        bindingRectColor = F3DImguiStyle::GetWarningColor();
        descTextColor = F3DImguiStyle::GetWarningColor();
        valueTextColor = F3DImguiStyle::GetWarningColor();
      }
      else
      {
        bindingTextColor = F3DImguiStyle::GetTextColor();
        bindingRectColor = F3DImguiStyle::GetMidColor();
        descTextColor = F3DImguiStyle::GetTextColor();
        valueTextColor = F3DImguiStyle::GetHighlightColor();
      }

      ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeightWithSpacing() + margin);

      ImGui::TableNextColumn();
      ImGui::TextColored(descTextColor, "%s", desc.c_str());

      ImGui::TableNextColumn();
      if (type == CheatSheetBindingType::CYCLIC)
      {
        ImGui::TextColored(valueTextColor, "< %s >", val.c_str());
      }
      else if (type == CheatSheetBindingType::NUMERICAL || type == CheatSheetBindingType::OTHER)
      {
        ImGui::TextColored(valueTextColor, "%s", val.c_str());
      }

      ImGui::TableNextColumn();

      ImVec2 topBindingCorner, bottomBindingCorner;
      std::vector<std::string> splittedBinding = ::SplitBindings(bind, '+');
      const float maxCursorPosX = ImGui::GetCursorPosX() + ImGui::GetColumnWidth();
      float posX = maxCursorPosX - ImGui::CalcTextSize(bind.c_str()).x - ImGui::GetScrollX() -
        ((splittedBinding.size() * 2) - 1) * ImGui::GetStyle().ItemSpacing.x;
      ImGui::SetCursorPosX(posX);
      for (const std::string& key : splittedBinding)
      {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->ChannelsSplit(2);
        drawList->ChannelsSetCurrent(1);
        ImGui::TextColored(bindingTextColor, "%s", key.c_str());
        drawList->ChannelsSetCurrent(0);
        topBindingCorner =
          ImVec2(ImGui::GetItemRectMin().x - margin, ImGui::GetItemRectMin().y - (margin * .5f));
        bottomBindingCorner =
          ImVec2(ImGui::GetItemRectMax().x + margin, ImGui::GetItemRectMax().y + (margin * .5f));
        drawList->AddRectFilled(
          topBindingCorner, bottomBindingCorner, ImColor(bindingRectColor), 5.f);
        drawList->ChannelsMerge();
        if (key != splittedBinding.back())
        {
          ImGui::SameLine();
          ImGui::Text("+");
        }
        ImGui::SameLine();
      }
    }

    ImGui::EndTable();
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::RenderFpsCounter()
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  constexpr float margin = F3DImguiStyle::GetDefaultMargin();

  std::string fpsString = std::to_string(this->FpsValue);
  fpsString += " fps";

  ImVec2 winSize = ImGui::CalcTextSize(fpsString.c_str());
  winSize.x += 2.f * ImGui::GetStyle().WindowPadding.x;
  winSize.y += 2.f * ImGui::GetStyle().WindowPadding.y;

  ImVec2 position(
    viewport->WorkSize.x - winSize.x - margin, viewport->WorkSize.y - winSize.y - margin);

  ::SetupNextWindow(position, winSize);
  ImGui::SetNextWindowBgAlpha(this->BackdropOpacity);

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

  ImGui::Begin("FpsCounter", nullptr, flags);
  ImGui::TextUnformatted(fpsString.c_str());
  ImGui::End();
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::RenderConsole(bool minimal)
{
  vtkF3DImguiConsole* console = vtkF3DImguiConsole::SafeDownCast(vtkOutputWindow::GetInstance());
  console->ShowConsole(minimal);
}

//----------------------------------------------------------------------------
void vtkF3DImguiActor::RenderConsoleBadge()
{
  vtkF3DImguiConsole* console = vtkF3DImguiConsole::SafeDownCast(vtkOutputWindow::GetInstance());
  console->ShowBadge();
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

//----------------------------------------------------------------------------
void vtkF3DImguiActor::SetDeltaTime(double time)
{
  ImGuiIO& io = ImGui::GetIO();
  io.DeltaTime = time;
}
