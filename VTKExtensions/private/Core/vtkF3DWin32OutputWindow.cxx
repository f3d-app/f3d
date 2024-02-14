#include "vtkF3DWin32OutputWindow.h"

#include <vtkObjectFactory.h>
#include <vtkWindows.h>

#include <codecvt>
#include <regex>

vtkStandardNewMacro(vtkF3DWin32OutputWindow);

//------------------------------------------------------------------------------
vtkF3DWin32OutputWindow::vtkF3DWin32OutputWindow()
{
  // Override base class behavior and set displayMode to default on creation
  this->SetDisplayModeToDefault();
}

// WindowTitle needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7460
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201207)
//------------------------------------------------------------------------------
const char* vtkF3DWin32OutputWindow::GetWindowTitle()
{
  return "F3D log window";
}
#endif

//------------------------------------------------------------------------------
int vtkF3DWin32OutputWindow::Initialize()
{
  int rc = this->Superclass::Initialize();

  // retrieve output window handle
  HWND hCurWnd = nullptr;
  do
  {
    hCurWnd = FindWindowExA(nullptr, hCurWnd, "vtkOutputWindow", nullptr);
    DWORD processID = 0;
    GetWindowThreadProcessId(hCurWnd, &processID);
    if (processID == GetCurrentProcessId())
    {
      break;
    }
  } while (hCurWnd != nullptr);

  if (hCurWnd == nullptr)
  {
    // cannot find window
    this->EditControlHandle = nullptr;
    return 0;
  }

  // find Edit control
  this->EditControlHandle = FindWindowExA(hCurWnd, nullptr, "Edit", nullptr);

  // retrieve f3d window handle
  HWND f3dCurWnd = nullptr;
  do
  {
    f3dCurWnd = FindWindowExA(nullptr, f3dCurWnd, "vtkOpenGL", nullptr);
    DWORD processID = 0;
    GetWindowThreadProcessId(f3dCurWnd, &processID);
    if (processID == GetCurrentProcessId())
    {
      break;
    }
  } while (f3dCurWnd != nullptr);

  // Put f3d window on top of output window
  if (f3dCurWnd)
  {
    BringWindowToTop(f3dCurWnd);
  }

  return rc;
}

//------------------------------------------------------------------------------
void vtkF3DWin32OutputWindow::DisplayText(const char* text)
{
  if (this->GetDisplayStream(this->GetCurrentMessageType()) == StreamType::Null)
  {
    return;
  }

  if (!this->Initialize())
  {
    return;
  }

  std::string str(text);
  str = std::regex_replace(str, std::regex("\n"), "\r\n");

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > converter;
  std::wstring wstr = converter.from_bytes(str);
  wstr += L"\r\n";

  HWND hWnd = static_cast<HWND>(this->EditControlHandle);

  // select end of text
  int index = GetWindowTextLength(hWnd);
  SendMessageW(hWnd, EM_SETSEL, (WPARAM)index, (LPARAM)index);

  // print text
  SendMessageW(hWnd, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(wstr.c_str()));

  // still print text to std::cout in
  // order to be able to test verbose outputs
  std::cout << text << "\n";
  std::cout.flush();
}

//------------------------------------------------------------------------------
void vtkF3DWin32OutputWindow::WaitForUser()
{
  if (getenv("DASHBOARD_TEST_FROM_CTEST") || getenv("CTEST_INTERACTIVE_DEBUG_MODE"))
  {
    // This ensure that WaitForUser will not do anything when using ctest
    return;
  }
  MessageBoxW(nullptr, L"Press OK to continue.", L"F3D", MB_ICONINFORMATION | MB_OK);
}
