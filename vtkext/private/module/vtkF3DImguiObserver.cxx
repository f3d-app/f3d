#include "vtkF3DImguiObserver.h"

#include "vtkF3DRenderPass.h"
#include "vtkF3DRenderer.h"

#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>

#include <imgui.h>

namespace
{
  ImGuiKey GetImGuiKeyFromKeySym(std::string_view&& keySym)
  {
    if (keySym == "Tab") return ImGuiKey_Tab;
    if (keySym == "Left") return ImGuiKey_LeftArrow;
    if (keySym == "Right") return ImGuiKey_RightArrow;
    if (keySym == "Up") return ImGuiKey_UpArrow;
    if (keySym == "Down") return ImGuiKey_DownArrow;
    if (keySym == "Prior") return ImGuiKey_PageUp;
    if (keySym == "Next") return ImGuiKey_PageDown;
    if (keySym == "Home") return ImGuiKey_Home;
    if (keySym == "End") return ImGuiKey_End;
    if (keySym == "Insert") return ImGuiKey_Insert;
    if (keySym == "Delete") return ImGuiKey_Delete;
    if (keySym == "Backspace") return ImGuiKey_Backspace;
    if (keySym == "space") return ImGuiKey_Space;
    if (keySym == "Return") return ImGuiKey_Enter;
    if (keySym == "Escape") return ImGuiKey_Escape;
    if (keySym == "Control_L") return ImGuiKey_LeftCtrl;
    if (keySym == "Shift_L") return ImGuiKey_LeftShift;
    if (keySym == "Alt_L") return ImGuiKey_LeftAlt;
    if (keySym == "Super_L") return ImGuiKey_LeftSuper;
    if (keySym == "Control_R") return ImGuiKey_RightCtrl;
    if (keySym == "Shift_R") return ImGuiKey_RightShift;
    if (keySym == "Alt_R") return ImGuiKey_RightAlt;
    if (keySym == "Super_R") return ImGuiKey_RightSuper;
    if (keySym == "Menu") return ImGuiKey_Menu;
    if (keySym == "0") return ImGuiKey_0;
    if (keySym == "1") return ImGuiKey_1;
    if (keySym == "2") return ImGuiKey_2;
    if (keySym == "3") return ImGuiKey_3;
    if (keySym == "4") return ImGuiKey_4;
    if (keySym == "5") return ImGuiKey_5;
    if (keySym == "6") return ImGuiKey_6;
    if (keySym == "7") return ImGuiKey_7;
    if (keySym == "8") return ImGuiKey_8;
    if (keySym == "9") return ImGuiKey_9;
    if (keySym == "a" || keySym == "A") return ImGuiKey_A;
    if (keySym == "b" || keySym == "B") return ImGuiKey_B;
    if (keySym == "c" || keySym == "C") return ImGuiKey_C;
    if (keySym == "d" || keySym == "D") return ImGuiKey_D;
    if (keySym == "e" || keySym == "E") return ImGuiKey_E;
    if (keySym == "f" || keySym == "F") return ImGuiKey_F;
    if (keySym == "g" || keySym == "G") return ImGuiKey_G;
    if (keySym == "h" || keySym == "H") return ImGuiKey_H;
    if (keySym == "i" || keySym == "I") return ImGuiKey_I;
    if (keySym == "j" || keySym == "J") return ImGuiKey_J;
    if (keySym == "k" || keySym == "K") return ImGuiKey_K;
    if (keySym == "l" || keySym == "L") return ImGuiKey_L;
    if (keySym == "m" || keySym == "M") return ImGuiKey_M;
    if (keySym == "n" || keySym == "N") return ImGuiKey_N;
    if (keySym == "o" || keySym == "O") return ImGuiKey_O;
    if (keySym == "p" || keySym == "P") return ImGuiKey_P;
    if (keySym == "q" || keySym == "Q") return ImGuiKey_Q;
    if (keySym == "r" || keySym == "R") return ImGuiKey_R;
    if (keySym == "s" || keySym == "S") return ImGuiKey_S;
    if (keySym == "t" || keySym == "T") return ImGuiKey_T;
    if (keySym == "u" || keySym == "U") return ImGuiKey_U;
    if (keySym == "v" || keySym == "V") return ImGuiKey_V;
    if (keySym == "w" || keySym == "W") return ImGuiKey_W;
    if (keySym == "x" || keySym == "X") return ImGuiKey_X;
    if (keySym == "y" || keySym == "Y") return ImGuiKey_Y;
    if (keySym == "z" || keySym == "Z") return ImGuiKey_Z;
    if (keySym == "F1") return ImGuiKey_F1;
    if (keySym == "F2") return ImGuiKey_F2;
    if (keySym == "F3") return ImGuiKey_F3;
    if (keySym == "F4") return ImGuiKey_F4;
    if (keySym == "F5") return ImGuiKey_F5;
    if (keySym == "F6") return ImGuiKey_F6;
    if (keySym == "F7") return ImGuiKey_F7;
    if (keySym == "F8") return ImGuiKey_F8;
    if (keySym == "F9") return ImGuiKey_F9;
    if (keySym == "F10") return ImGuiKey_F10;
    if (keySym == "F11") return ImGuiKey_F11;
    if (keySym == "F12") return ImGuiKey_F12;
    if (keySym == "F13") return ImGuiKey_F13;
    if (keySym == "F14") return ImGuiKey_F14;
    if (keySym == "F15") return ImGuiKey_F15;
    if (keySym == "F16") return ImGuiKey_F16;
    if (keySym == "F17") return ImGuiKey_F17;
    if (keySym == "F18") return ImGuiKey_F18;
    if (keySym == "F19") return ImGuiKey_F19;
    if (keySym == "F20") return ImGuiKey_F20;
    if (keySym == "F21") return ImGuiKey_F21;
    if (keySym == "F22") return ImGuiKey_F22;
    if (keySym == "F23") return ImGuiKey_F23;
    if (keySym == "F24") return ImGuiKey_F24;
    if (keySym == "apostrophe") return ImGuiKey_Apostrophe;
    if (keySym == "comma") return ImGuiKey_Comma;
    if (keySym == "minus") return ImGuiKey_Minus;
    if (keySym == "period") return ImGuiKey_Period;
    if (keySym == "slash") return ImGuiKey_Slash;
    if (keySym == "semicolon") return ImGuiKey_Semicolon;
    if (keySym == "equal") return ImGuiKey_Equal;
    if (keySym == "bracketleft") return ImGuiKey_LeftBracket;
    if (keySym == "backslash") return ImGuiKey_Backslash;
    if (keySym == "bracketright") return ImGuiKey_RightBracket;
    if (keySym == "grave") return ImGuiKey_GraveAccent;
    if (keySym == "Caps_Lock") return ImGuiKey_CapsLock;
    if (keySym == "Scroll_Lock") return ImGuiKey_ScrollLock;
    if (keySym == "Num_Lock") return ImGuiKey_NumLock;
    if (keySym == "Snapshot") return ImGuiKey_PrintScreen;
    if (keySym == "Pause") return ImGuiKey_Pause;
    if (keySym == "KP_0") return ImGuiKey_Keypad0;
    if (keySym == "KP_1") return ImGuiKey_Keypad1;
    if (keySym == "KP_2") return ImGuiKey_Keypad2;
    if (keySym == "KP_3") return ImGuiKey_Keypad3;
    if (keySym == "KP_4") return ImGuiKey_Keypad4;
    if (keySym == "KP_5") return ImGuiKey_Keypad5;
    if (keySym == "KP_6") return ImGuiKey_Keypad6;
    if (keySym == "KP_7") return ImGuiKey_Keypad7;
    if (keySym == "KP_8") return ImGuiKey_Keypad8;
    if (keySym == "KP_9") return ImGuiKey_Keypad9;

    return ImGuiKey::ImGuiKey_None;
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

  int sz[2];
  int p[2];
  that->GetEventPosition(p);
  that->GetSize(sz);
  ImGuiIO& io = ImGui::GetIO();
  io.AddMousePosEvent(static_cast<float>(p[0]), static_cast<float>(sz[1] - p[1] - 1));

  this->RenderUI(that);

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
bool vtkF3DImguiObserver::KeyPress(vtkObject* caller, unsigned long, void*)
{
  vtkRenderWindowInteractor* that = static_cast<vtkRenderWindowInteractor*>(caller);

  ImGuiIO& io = ImGui::GetIO();

  io.AddKeyEvent(ImGuiMod_Ctrl, that->GetControlKey() == 1);
  io.AddKeyEvent(ImGuiMod_Shift, that->GetShiftKey() == 1);
  io.AddKeyEvent(ImGuiMod_Alt, that->GetAltKey() == 1);

  io.AddKeyEvent(::GetImGuiKeyFromKeySym(that->GetKeySym()), true);
  io.AddInputCharacter(that->GetKeyCode());

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
  interactor->AddObserver(vtkCommand::LeftButtonPressEvent, this, &vtkF3DImguiObserver::MouseLeftPress, 2.f);
  interactor->AddObserver(vtkCommand::LeftButtonReleaseEvent, this, &vtkF3DImguiObserver::MouseLeftRelease, 2.f);
  interactor->AddObserver(vtkCommand::RightButtonPressEvent, this, &vtkF3DImguiObserver::MouseRightPress, 2.f);
  interactor->AddObserver(vtkCommand::RightButtonReleaseEvent, this, &vtkF3DImguiObserver::MouseRightRelease, 2.f);
  interactor->AddObserver(vtkCommand::MouseWheelForwardEvent, this, &vtkF3DImguiObserver::MouseWheelForward, 2.f);
  interactor->AddObserver(vtkCommand::MouseWheelBackwardEvent, this, &vtkF3DImguiObserver::MouseWheelBackward, 2.f);
  interactor->AddObserver(vtkCommand::KeyPressEvent, this, &vtkF3DImguiObserver::KeyPress, 2.f);
  interactor->AddObserver(vtkCommand::KeyReleaseEvent, this, &vtkF3DImguiObserver::KeyRelease, 2.f);
}
