#include "Config.h"
#include "F3DLoader.h"

#include "vtkF3DObjectFactory.h"

int main(int argc, char** argv)
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

  // instanciate our own polydata mapper and output windows
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  F3DLoader loader;
  return loader.Start(argc, argv);
}

#ifdef _WIN32
#include <Windows.h>
#include <clocale>
#include <locale>
#include <string>
#include <codecvt>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
  // Set console code page to UTF-8 so console known how to interpret string data
  SetConsoleOutputCP(CP_UTF8);

  // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
  setvbuf(stdout, nullptr, _IOFBF, 1000);
  setvbuf(stderr, nullptr, _IOFBF, 1000);

  std::setlocale(LC_ALL, "en_US.utf8");

  int argc;
  wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &argc);

  char** argv = new char*[argc];
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conversion;

  for (int i = 0; i < argc; i++)
  {
    std::string mbs = conversion.to_bytes(wargv[i]);
    argv[i] = new char[mbs.size() + 1];
    std::copy(mbs.begin(), mbs.end(), argv[i]);
    argv[i][mbs.size()] = 0;
  }

  int rc = main(argc, argv);

  for (int i = 0; i < argc; i++)
  {
    delete[] argv[i];
  }
  delete[] argv;

  return rc;
}
#endif
