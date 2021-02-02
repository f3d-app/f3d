#include "vtkF3DWin32OutputWindow.h"

#include <vtkObjectFactory.h>
#include <vtkUnicodeString.h>

#include <Windows.h>

#include <codecvt>
#include <regex>

extern HWND vtkWin32OutputWindowOutputWindow;

vtkStandardNewMacro(vtkF3DWin32OutputWindow);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201207)
const char* vtkF3DWin32OutputWindow::GetWindowTitle()
{
  return "F3D log window";
}
#endif

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

  std::vector<vtkTypeUInt16> utf16data = vtkUnicodeString::from_utf8(someText).utf16_str();

  int index = GetWindowTextLength(vtkWin32OutputWindowOutputWindow);
  SetFocus(vtkWin32OutputWindowOutputWindow);

  // select end of text
  SendMessageW(vtkWin32OutputWindowOutputWindow, EM_SETSEL, (WPARAM)index, (LPARAM)index);

  // print text
  SendMessageW(vtkWin32OutputWindowOutputWindow, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(wstr.c_str()));
}
