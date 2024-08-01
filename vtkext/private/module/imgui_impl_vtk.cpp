#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_impl_vtk.h"

#include <vtkOpenGLState.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkTextureObject.h>
#include <vtkOpenGLBufferObject.h>
#include <vtkOpenGLShaderCache.h>
#include <vtkOpenGLVertexArrayObject.h>
#include <vtkShader.h>
#include <vtkShaderProgram.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtk_glew.h>

#include "vtkF3DImguiFS.h"
#include "vtkF3DImguiVS.h"

struct ImGui_ImplVTK_Data
{
    vtkSmartPointer<vtkOpenGLRenderWindow> RenderWindow;
    vtkSmartPointer<vtkTextureObject> FontTexture;
    vtkSmartPointer<vtkOpenGLVertexArrayObject> VertexArray;
    vtkSmartPointer<vtkOpenGLBufferObject> VertexBuffer;
    vtkSmartPointer<vtkOpenGLBufferObject> IndexBuffer;
    vtkSmartPointer<vtkShaderProgram> Program;
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
static ImGui_ImplVTK_Data* ImGui_ImplVTK_GetBackendData()     { return ImGui::GetCurrentContext() ? (ImGui_ImplVTK_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr; }

// Render function.
void ImGui_ImplVTK_RenderDrawData(ImDrawData* draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    ImGui_ImplVTK_Data* bd = ImGui_ImplVTK_GetBackendData();

    vtkOpenGLState* state = bd->RenderWindow->GetState();

    vtkOpenGLState::ScopedglScissor save_scissorbox(state);
    vtkOpenGLState::ScopedglBlendFuncSeparate save_blendfunc(state);
    vtkOpenGLState::ScopedglEnableDisable save_blend(state, GL_BLEND);
    vtkOpenGLState::ScopedglEnableDisable save_cull(state, GL_CULL_FACE);
    vtkOpenGLState::ScopedglEnableDisable save_depth(state, GL_DEPTH_TEST);
    vtkOpenGLState::ScopedglEnableDisable save_stencil(state, GL_STENCIL_TEST);
    vtkOpenGLState::ScopedglEnableDisable save_scissor(state, GL_SCISSOR_TEST);

    state->vtkglEnable(GL_BLEND);
    state->vtkglBlendEquation(GL_FUNC_ADD);
    state->vtkglBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    state->vtkglDisable(GL_CULL_FACE);
    state->vtkglDisable(GL_DEPTH_TEST);
    state->vtkglDisable(GL_STENCIL_TEST);
    state->vtkglEnable(GL_SCISSOR_TEST);

    bd->RenderWindow->GetShaderCache()->ReadyShaderProgram(bd->Program);

    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    float projMatrix[16] =
    {
        2.0f/(R-L),   0.0f,         0.0f,   0.0f,
        0.0f,         2.0f/(T-B),   0.0f,   0.0f,
        0.0f,         0.0f,        -1.0f,   0.0f,
        (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f,
    };

    bd->Program->SetUniformMatrix4x4v("ProjMtx", 1, projMatrix);

    bd->VertexArray->Bind();
    bd->VertexBuffer->Bind();
    bd->IndexBuffer->Bind();

    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        bd->VertexBuffer->Upload(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, vtkOpenGLBufferObject::ArrayBuffer);
        bd->IndexBuffer->Upload(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, vtkOpenGLBufferObject::ElementArrayBuffer);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            // Project scissor/clipping rectangles into framebuffer space
            ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
            ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
            if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                continue;

            // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
            state->vtkglScissor((int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y));

            // Bind texture, Draw
            bd->FontTexture->Bind();

            glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
        }
    }
}

bool ImGui_ImplVTK_CreateDeviceObjects()
{
    // Build texture atlas
    ImGui_ImplVTK_Data* bd = ImGui_ImplVTK_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    bd->FontTexture = vtkSmartPointer<vtkTextureObject>::New();
    bd->FontTexture->SetContext(bd->RenderWindow);
    bd->FontTexture->Create2DFromRaw(width, height, 4, VTK_UNSIGNED_CHAR, pixels);

    // Store our identifier
    io.Fonts->SetTexID((ImTextureID)(intptr_t)bd->FontTexture.Get());

    bd->VertexBuffer = vtkSmartPointer<vtkOpenGLBufferObject>::New();
    bd->VertexBuffer->SetUsage(vtkOpenGLBufferObject::StreamDraw);
    bd->VertexBuffer->GenerateBuffer(vtkOpenGLBufferObject::ArrayBuffer);

    bd->IndexBuffer = vtkSmartPointer<vtkOpenGLBufferObject>::New();
    bd->IndexBuffer->SetUsage(vtkOpenGLBufferObject::StreamDraw);
    bd->VertexBuffer->GenerateBuffer(vtkOpenGLBufferObject::ElementArrayBuffer);

    bd->Program = bd->RenderWindow->GetShaderCache()->ReadyShaderProgram(vtkF3DImguiVS, vtkF3DImguiFS, nullptr);
    bd->Program->Print(cout);

    bd->VertexArray = vtkSmartPointer<vtkOpenGLVertexArrayObject>::New();
    bd->VertexArray->Bind();
    bd->VertexArray->AddAttributeArray(bd->Program, bd->VertexBuffer , "Position", 0, sizeof(ImDrawVert), VTK_FLOAT, 2, false);
    bd->VertexArray->AddAttributeArray(bd->Program, bd->VertexBuffer , "UV", 8, sizeof(ImDrawVert), VTK_FLOAT, 2, false);
    bd->VertexArray->AddAttributeArray(bd->Program, bd->VertexBuffer , "Color", 16, sizeof(ImDrawVert), VTK_UNSIGNED_CHAR, 4, true);

    return true;
}

void ImGui_ImplVTK_InvalidateDeviceObjects()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplVTK_Data* bd = ImGui_ImplVTK_GetBackendData();
    if (bd->FontTexture)
    {
        io.Fonts->SetTexID(0);
        bd->FontTexture->ReleaseGraphicsResources(bd->RenderWindow);
        bd->FontTexture = nullptr;
    }

    if (bd->VertexBuffer)
    {
        bd->VertexBuffer = nullptr;
    }

    if (bd->IndexBuffer)
    {
        bd->IndexBuffer = nullptr;
    }

    if (bd->Program)
    {
        bd->Program->ReleaseGraphicsResources(bd->RenderWindow);
        bd->Program = nullptr;
    }
}

bool ImGui_ImplVTK_Init(vtkOpenGLRenderWindow* renWin)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    ImGui_ImplVTK_Data* bd = IM_NEW(ImGui_ImplVTK_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = io.BackendRendererName = "imgui_impl_vtk";
 
    bd->RenderWindow = renWin;

    return true;
}

void ImGui_ImplVTK_Shutdown()
{
    ImGui_ImplVTK_Data* bd = ImGui_ImplVTK_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplVTK_InvalidateDeviceObjects();
    io.BackendPlatformName = io.BackendRendererName = nullptr;
    io.BackendPlatformUserData = nullptr;
    IM_DELETE(bd);
}

void ImGui_ImplVTK_NewFrame()
{
    ImGui_ImplVTK_Data* bd = ImGui_ImplVTK_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplVTK_Init()?");

    if (!bd->FontTexture)
        ImGui_ImplVTK_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    int* size = bd->RenderWindow->GetSize();
    io.DisplaySize = ImVec2((float)size[0], (float)size[1]);
}

void ImGui_ImplVTK_AddObservers(vtkRenderWindowInteractor* interactor)
{
    vtkNew<vtkCallbackCommand> mouseMoveCB;
    mouseMoveCB->SetClientData(interactor);
    mouseMoveCB->PassiveObserverOn();
    mouseMoveCB->SetCallback(
      [](vtkObject*, unsigned long, void* clientData, void* callData)
      {
        vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(clientData);

        int sz[2];
        int p[2];
        that->GetEventPosition(p);
        that->GetSize(sz);
        ImGuiIO& io = ImGui::GetIO();
        std::cout << p[0] << " " << p[1] << std::endl;
        io.AddMousePosEvent(static_cast<float>(p[0]), static_cast<float>(sz[1] - p[1] - 1));
        
        // calling Frame() here instead of Render() refresh only the UI
        that->GetRenderWindow()->Frame();
      });
    interactor->AddObserver(vtkCommand::MouseMoveEvent, mouseMoveCB);

    vtkNew<vtkCallbackCommand> mouseleftpressCB;
    mouseleftpressCB->SetClientData(interactor);
    mouseleftpressCB->PassiveObserverOn();
    mouseleftpressCB->SetCallback(
      [](vtkObject*, unsigned long, void* clientData, void* callData)
      {
        vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(clientData);

        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);

        // calling Frame() here instead of Render() refresh only the UI
        that->GetRenderWindow()->Frame();
      });
    interactor->AddObserver(vtkCommand::LeftButtonPressEvent, mouseleftpressCB);

    vtkNew<vtkCallbackCommand> mouseleftreleaseCB;
    mouseleftreleaseCB->SetClientData(interactor);
    mouseleftreleaseCB->PassiveObserverOn();
    mouseleftreleaseCB->SetCallback(
      [](vtkObject*, unsigned long, void* clientData, void* callData)
      {
        vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(clientData);

        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);

        // calling Frame() here instead of Render() refresh only the UI
        that->GetRenderWindow()->Frame();
      });
    interactor->AddObserver(vtkCommand::LeftButtonReleaseEvent, mouseleftreleaseCB, 1.f);
}

//-----------------------------------------------------------------------------

#endif // #ifndef IMGUI_DISABLE
