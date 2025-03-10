#include "vtkF3DImguiObserver.h"

#include "vtkF3DRenderPass.h"
#include "vtkF3DRenderer.h"

#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkVersion.h>

#include <imgui.h>

#include <unordered_map>

namespace
{
ImGuiKey GetImGuiKeyFromKeySym(std::string_view&& keySym)
{
  // clang-format off
    static const std::unordered_map<std::string_view, ImGuiKey> keySymToImGuiKey =
    {
      { "Tab", ImGuiKey_Tab },
      { "Left", ImGuiKey_LeftArrow },
      { "Right", ImGuiKey_RightArrow },
      { "Up", ImGuiKey_UpArrow },
      { "Down", ImGuiKey_DownArrow },
      { "Prior", ImGuiKey_PageUp },
      { "Next", ImGuiKey_PageDown },
      { "Home", ImGuiKey_Home },
      { "End", ImGuiKey_End },
      { "Insert", ImGuiKey_Insert },
      { "Delete", ImGuiKey_Delete },
      { "BackSpace", ImGuiKey_Backspace },
// https://gitlab.kitware.com/vtk/vtk/-/merge_requests/11738
#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 4, 20241210)
      { "Backspace", ImGuiKey_Backspace },
#endif
      { "space", ImGuiKey_Space },
      { "Return", ImGuiKey_Enter },
      { "Escape", ImGuiKey_Escape },
      { "Control_L", ImGuiKey_LeftCtrl },
      { "Shift_L", ImGuiKey_LeftShift },
      { "Alt_L", ImGuiKey_LeftAlt },
      { "Super_L", ImGuiKey_LeftSuper },
      { "Control_R", ImGuiKey_RightCtrl },
      { "Shift_R", ImGuiKey_RightShift },
      { "Alt_R", ImGuiKey_RightAlt },
      { "Super_R", ImGuiKey_RightSuper },
      { "Menu", ImGuiKey_Menu },
      { "0", ImGuiKey_0 },
      { "1", ImGuiKey_1 },
      { "2", ImGuiKey_2 },
      { "3", ImGuiKey_3 },
      { "4", ImGuiKey_4 },
      { "5", ImGuiKey_5 },
      { "6", ImGuiKey_6 },
      { "7", ImGuiKey_7 },
      { "8", ImGuiKey_8 },
      { "9", ImGuiKey_9 },
      { "a", ImGuiKey_A },
      { "b", ImGuiKey_B },
      { "c", ImGuiKey_C },
      { "d", ImGuiKey_D },
      { "e", ImGuiKey_E },
      { "f", ImGuiKey_F },
      { "g", ImGuiKey_G },
      { "h", ImGuiKey_H },
      { "i", ImGuiKey_I },
      { "j", ImGuiKey_J },
      { "k", ImGuiKey_K },
      { "l", ImGuiKey_L },
      { "m", ImGuiKey_M },
      { "n", ImGuiKey_N },
      { "o", ImGuiKey_O },
      { "p", ImGuiKey_P },
      { "q", ImGuiKey_Q },
      { "r", ImGuiKey_R },
      { "s", ImGuiKey_S },
      { "t", ImGuiKey_T },
      { "u", ImGuiKey_U },
      { "v", ImGuiKey_V },
      { "w", ImGuiKey_W },
      { "x", ImGuiKey_X },
      { "y", ImGuiKey_Y },
      { "z", ImGuiKey_Z },
      { "A", ImGuiKey_A },
      { "B", ImGuiKey_B },
      { "C", ImGuiKey_C },
      { "D", ImGuiKey_D },
      { "E", ImGuiKey_E },
      { "F", ImGuiKey_F },
      { "G", ImGuiKey_G },
      { "H", ImGuiKey_H },
      { "I", ImGuiKey_I },
      { "J", ImGuiKey_J },
      { "K", ImGuiKey_K },
      { "L", ImGuiKey_L },
      { "M", ImGuiKey_M },
      { "N", ImGuiKey_N },
      { "O", ImGuiKey_O },
      { "P", ImGuiKey_P },
      { "Q", ImGuiKey_Q },
      { "R", ImGuiKey_R },
      { "S", ImGuiKey_S },
      { "T", ImGuiKey_T },
      { "U", ImGuiKey_U },
      { "V", ImGuiKey_V },
      { "W", ImGuiKey_W },
      { "X", ImGuiKey_X },
      { "Y", ImGuiKey_Y },
      { "Z", ImGuiKey_Z },
      { "F1", ImGuiKey_F1 },
      { "F2", ImGuiKey_F2 },
      { "F3", ImGuiKey_F3 },
      { "F4", ImGuiKey_F4 },
      { "F5", ImGuiKey_F5 },
      { "F6", ImGuiKey_F6 },
      { "F7", ImGuiKey_F7 },
      { "F8", ImGuiKey_F8 },
      { "F9", ImGuiKey_F9 },
      { "F10", ImGuiKey_F10 },
      { "F11", ImGuiKey_F11 },
      { "F12", ImGuiKey_F12 },
      { "F13", ImGuiKey_F13 },
      { "F14", ImGuiKey_F14 },
      { "F15", ImGuiKey_F15 },
      { "F16", ImGuiKey_F16 },
      { "F17", ImGuiKey_F17 },
      { "F18", ImGuiKey_F18 },
      { "F19", ImGuiKey_F19 },
      { "F20", ImGuiKey_F20 },
      { "F21", ImGuiKey_F21 },
      { "F22", ImGuiKey_F22 },
      { "F23", ImGuiKey_F23 },
      { "F24", ImGuiKey_F24 },
      { "apostrophe", ImGuiKey_Apostrophe },
      { "comma", ImGuiKey_Comma },
      { "minus", ImGuiKey_Minus },
      { "period", ImGuiKey_Period },
      { "slash", ImGuiKey_Slash },
      { "semicolon", ImGuiKey_Semicolon },
      { "equal", ImGuiKey_Equal },
      { "bracketleft", ImGuiKey_LeftBracket },
      { "backslash", ImGuiKey_Backslash },
      { "bracketright", ImGuiKey_RightBracket },
      { "grave", ImGuiKey_GraveAccent },
      { "Caps_Lock", ImGuiKey_CapsLock },
      { "Scroll_Lock", ImGuiKey_ScrollLock },
      { "Num_Lock", ImGuiKey_NumLock },
      { "Snapshot", ImGuiKey_PrintScreen },
      { "Pause", ImGuiKey_Pause },
      { "KP_0", ImGuiKey_Keypad0 },
      { "KP_1", ImGuiKey_Keypad1 },
      { "KP_2", ImGuiKey_Keypad2 },
      { "KP_3", ImGuiKey_Keypad3 },
      { "KP_4", ImGuiKey_Keypad4 },
      { "KP_5", ImGuiKey_Keypad5 },
      { "KP_6", ImGuiKey_Keypad6 },
      { "KP_7", ImGuiKey_Keypad7 },
      { "KP_8", ImGuiKey_Keypad8 },
      { "KP_9", ImGuiKey_Keypad9 }
    };
  // clang-format on

  auto it = keySymToImGuiKey.find(keySym);

  if (it == keySymToImGuiKey.end())
  {
    return ImGuiKey::ImGuiKey_None;
  }

  return it->second;
}
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DImguiObserver);

//----------------------------------------------------------------------------
void vtkF3DImguiObserver::RenderUI(vtkRenderWindowInteractor* interactor)
{
  vtkRenderWindow* renWin = interactor->GetRenderWindow();
  vtkF3DRenderer* ren = vtkF3DRenderer::SafeDownCast(renWin->GetRenderers()->GetFirstRenderer());
  vtkInformation* info = ren->GetInformation();
  info->Set(vtkF3DRenderPass::RENDER_UI_ONLY(), 1);
  renWin->Render();
  info->Remove(vtkF3DRenderPass::RENDER_UI_ONLY());
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::MouseMove(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  int* p = that->GetEventPosition();
  int* sz = that->GetRenderWindow()->GetSize();
  ImGuiIO& io = ImGui::GetIO();
  io.AddMousePosEvent(static_cast<float>(p[0]), static_cast<float>(sz[1] - p[1] - 1));
  // RenderUI is not called here on purpose to avoid too frequent UI draw
  // The event loop is taking care of it
  return io.WantCaptureMouse;
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::MouseLeftPress(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  this->RenderUI(that);
  return io.WantCaptureMouse;
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::MouseLeftRelease(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
  this->RenderUI(that);
  return io.WantCaptureMouse;
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::MouseRightPress(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseButtonEvent(ImGuiMouseButton_Right, true);
  this->RenderUI(that);
  return io.WantCaptureMouse;
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::MouseRightRelease(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseButtonEvent(ImGuiMouseButton_Right, false);
  this->RenderUI(that);
  return io.WantCaptureMouse;
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::MouseWheelForward(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseWheelEvent(0.f, 1.f);
  this->RenderUI(that);
  return io.WantCaptureMouse;
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::MouseWheelBackward(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseWheelEvent(0.f, -1.f);
  this->RenderUI(that);
  return io.WantCaptureMouse;
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::Char(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  ImGuiIO& io = ImGui::GetIO();
  io.AddInputCharacter(that->GetKeyCode());
  this->RenderUI(that);
  return io.WantCaptureKeyboard;
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::KeyPress(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  ImGuiIO& io = ImGui::GetIO();
  io.AddKeyEvent(ImGuiMod_Ctrl, that->GetControlKey() == 1);
  io.AddKeyEvent(ImGuiMod_Shift, that->GetShiftKey() == 1);
  io.AddKeyEvent(ImGuiMod_Alt, that->GetAltKey() == 1);
  io.AddKeyEvent(::GetImGuiKeyFromKeySym(that->GetKeySym()), true);
  this->RenderUI(that);
  return io.WantCaptureKeyboard;
}

//----------------------------------------------------------------------------
bool vtkF3DImguiObserver::KeyRelease(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);
  ImGuiIO& io = ImGui::GetIO();
  io.AddKeyEvent(ImGuiMod_Ctrl, that->GetControlKey() == 1);
  io.AddKeyEvent(ImGuiMod_Shift, that->GetShiftKey() == 1);
  io.AddKeyEvent(ImGuiMod_Alt, that->GetAltKey() == 1);
  io.AddKeyEvent(::GetImGuiKeyFromKeySym(that->GetKeySym()), false);
  this->RenderUI(that);
  return io.WantCaptureKeyboard;
}

//----------------------------------------------------------------------------
void vtkF3DImguiObserver::InstallObservers(vtkRenderWindowInteractor* interactor)
{
  interactor->AddObserver(vtkCommand::MouseMoveEvent, this, &vtkF3DImguiObserver::MouseMove, 2.f);
  interactor->AddObserver(
    vtkCommand::LeftButtonPressEvent, this, &vtkF3DImguiObserver::MouseLeftPress, 2.f);
  interactor->AddObserver(
    vtkCommand::LeftButtonReleaseEvent, this, &vtkF3DImguiObserver::MouseLeftRelease, 2.f);
  interactor->AddObserver(
    vtkCommand::RightButtonPressEvent, this, &vtkF3DImguiObserver::MouseRightPress, 2.f);
  interactor->AddObserver(
    vtkCommand::RightButtonReleaseEvent, this, &vtkF3DImguiObserver::MouseRightRelease, 2.f);
  interactor->AddObserver(
    vtkCommand::MouseWheelForwardEvent, this, &vtkF3DImguiObserver::MouseWheelForward, 2.f);
  interactor->AddObserver(
    vtkCommand::MouseWheelBackwardEvent, this, &vtkF3DImguiObserver::MouseWheelBackward, 2.f);
  interactor->AddObserver(vtkCommand::KeyPressEvent, this, &vtkF3DImguiObserver::KeyPress, 2.f);
  interactor->AddObserver(vtkCommand::KeyReleaseEvent, this, &vtkF3DImguiObserver::KeyRelease, 2.f);
  interactor->AddObserver(vtkCommand::CharEvent, this, &vtkF3DImguiObserver::Char, 2.f);
}
