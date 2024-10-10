#pragma once

#include "imgui.h"      // IMGUI_IMPL_API

#ifndef IMGUI_DISABLE

class vtkOpenGLRenderWindow;
class vtkRenderWindowInteractor;

IMGUI_IMPL_API bool     ImGui_ImplVTK_Init(vtkOpenGLRenderWindow* renWin);
IMGUI_IMPL_API void     ImGui_ImplVTK_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplVTK_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplVTK_RenderDrawData(ImDrawData* draw_data);

IMGUI_IMPL_API void     ImGui_ImplVTK_AddObservers(vtkRenderWindowInteractor* renWin);

#endif // #ifndef IMGUI_DISABLE
