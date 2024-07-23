#include "vtkF3DUIRenderWindow.h"

#include "vtkF3DRenderer.h"
#include "vtkRendererCollection.h"

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

  io.Fonts->Clear();
  io.Fonts->AddFontFromFileTTF("/home/michael/dev/f3d/resources/Inter-Regular.ttf", 16);
  io.Fonts->Build();

  ImGuiStyle* style = &ImGui::GetStyle();
  style->GrabRounding = 4.0f;
  style->WindowRounding = 8.f;
  style->WindowBorderSize = 0.f;
  style->WindowPadding = ImVec2(10, 10);

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
  this->Superclass::BlitDisplayFramebuffersToHardware();

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2((float)this->Size[0], (float)this->Size[1]);

#ifdef ogl_
  ImGui_ImplOpenGL3_NewFrame();
#else
  ImGui_ImplVTK_NewFrame();
#endif
  ImGui::NewFrame();

  int marginLeft = 20;
  int marginTB = 30;
  ImGui::SetNextWindowPos(ImVec2(marginLeft, marginTB));
  ImGui::SetNextWindowSize(ImVec2(300, this->Size[1] - 2 * marginTB - 1));

  ImGui::Begin("Cheatsheet", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(this->Renderers->GetFirstRenderer());

  auto b = [](bool v) { return v ? "ON" : "OFF"; };

  ImGui::Button("test");

  ImGui::SeparatorText("Options");
  ImGui::Text("W: Cycle animation [%s]", "N/A");
  ImGui::Text("P: Translucency support [%s]", b(true));
  ImGui::Text("Q: Ambient occlusion [%s]", b(true));
  ImGui::Text("A: Anti-aliasing [%s]", b(true));
  ImGui::Text("T: Tone mapping [%s]", b(true));
  ImGui::Text("E: Edge visibility [%s]", b(true));
  ImGui::Text("X: Axis [%s]", b(true));
  ImGui::Text("G: Grid [%s]", b(true));
  ImGui::Text("N: File name [%s]", b(true));
  ImGui::Text("M: Metadata [%s]", b(true));
  ImGui::Text("Z: FPS Timer [%s]", b(true));
  ImGui::Text("U: Blur background [%s]", b(true));
  ImGui::Text("K: Trackball interaction [%s]", b(true));
  ImGui::Text("F: HDRI ambient lighting [%s]", b(true));
  ImGui::Text("J: HDRI skybox [%s]", b(true));
  ImGui::Text("L: Light (increase, shift+L: decrease) [%.2f]", 1.f);

  ImGui::SeparatorText("Hotkeys");
  ImGui::Text("H: Cheat sheet");
  ImGui::Text("?: Print scene descr to terminal");
  ImGui::Text("ESC : Quit ");
  ImGui::Text("SPACE: Play animation if any");
  ImGui::Text("LEFT : Previous file ");
  ImGui::Text("RIGHT: Next file ");
  ImGui::Text("UP  : Reload current file ");
  ImGui::Text("DOWN : Add files from dir of current file");
  ImGui::Text("ENTER: Reset camera to initial parameters");
  ImGui::Text("Drop  : Load dropped file, folder or HDRI");

  ImGui::SeparatorText("Camera");
  ImGui::Text("1: Front View camera");
  ImGui::Text("3: Right View camera");
  ImGui::Text("5: Toggle Orthographic Projection [%s]", b(false));
  ImGui::Text("7: Top View camera");
  ImGui::Text("9: Isometric View camera");

  ImGui::End();

  ImGui::Render();

#ifdef ogl_
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#else
  ImGui_ImplVTK_RenderDrawData(ImGui::GetDrawData());
#endif
}
