#include "vtkF3DWin32OutputWindow.h"

#include <vtkObjectFactory.h>
#include <vtkWindows.h>

#include <codecvt>
#include <regex>

vtkStandardNewMacro(vtkF3DWin32OutputWindow);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201207)
const char* vtkF3DWin32OutputWindow::GetWindowTitle()
{
  return "F3D log window";
}
#endif

int vtkF3DWin32OutputWindow::Initialize()
{
  int rc = this->Superclass::Initialize();

  // retrieve window handle
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
  }
  while (hCurWnd != nullptr);

  if (hCurWnd == nullptr)
  {
    // cannot find window
    this->EditControlHandle = nullptr;
    return 0;
  }

  // find Edit control
  this->EditControlHandle = FindWindowExA(hCurWnd, nullptr, "Edit", nullptr);

  return rc;
}

void vtkF3DWin32OutputWindow::DisplayText(const char* someText)
{
  if (!this->Initialize())
  {
    return;
  }

  std::string str(someText);
  str = std::regex_replace(str, std::regex("\n"), "\r\n");

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wstr = converter.from_bytes(str);
  wstr += L"\r\n";

  HWND hWnd = static_cast<HWND>(this->EditControlHandle);

  // select end of text
  int index = GetWindowTextLength(hWnd);
  SendMessageW(hWnd, EM_SETSEL, (WPARAM)index, (LPARAM)index);

  // print text
  SendMessageW(hWnd, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(wstr.c_str()));

  // print text to cout for testing purpose
  std::cout<<someText;
  std::cout.flush();
}
