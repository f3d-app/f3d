#include "vtkF3DUIRenderWindow.h"

#include <vtkObjectFactory.h>

#include "imgui.h"

//#define ogl_

#ifdef ogl_
#include "imgui_impl_opengl3.h"
#else
#include "imgui_impl_vtk.h"
#endif

vtkStandardNewMacro(vtkF3DUIRenderWindow);

vtkF3DUIRenderWindow::~vtkF3DUIRenderWindow()
{
#ifdef ogl_
  ImGui_ImplOpenGL3_Shutdown();
#else
  ImGui_ImplVTK_Shutdown();
#endif

  ImGuiIO& io = ImGui::GetIO();
  io.BackendPlatformName = io.BackendRendererName = nullptr;
  io.BackendPlatformUserData = nullptr;

  ImGui::DestroyContext();
}

void vtkF3DUIRenderWindow::OpenGLInitContext()
{
  this->Superclass::OpenGLInitContext();

  std::cout << "init" << std::endl;

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

  // Setup backend capabilities flags
  io.BackendPlatformUserData = nullptr;
  io.BackendPlatformName = io.BackendRendererName = "imgui_impl_vtk";

#ifdef ogl_
  ImGui_ImplOpenGL3_Init();
#else
  ImGui_ImplVTK_Init(this);
#endif
}

void vtkF3DUIRenderWindow::BlitDisplayFramebuffersToHardware()
{
  std::cout << "blit!\n";

  this->Superclass::BlitDisplayFramebuffersToHardware();

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2((float)this->Size[0], (float)this->Size[1]);

#ifdef ogl_
  ImGui_ImplOpenGL3_NewFrame();
#else
  ImGui_ImplVTK_NewFrame();
#endif
  ImGui::NewFrame();

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(300, 300));

  bool opened = true;
  ImGui::Begin("Cheatsheet !", &opened);

  if (ImGui::Button("Enable Grid", ImVec2(280, 20)))
  {
    std::cout << "click!" << std::endl;
  }

  if (ImGui::Button("Enable Depth Peeling", ImVec2(280, 20)))
  {
    std::cout << "click!" << std::endl;
  }

  ImGui::End();

  ImGui::Render();

#ifdef ogl_
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else
  ImGui_ImplVTK_RenderDrawData(ImGui::GetDrawData());
#endif
}
