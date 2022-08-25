#include "F3DException.h"
#include "F3DStarter.h"

#include "log.h"

int main(int argc, char** argv)
{
  int res = EXIT_FAILURE;

  try
  {
    F3DStarter starter;
    res = starter.Start(argc, argv);
  }
  catch (const F3DExNoProcess&)
  {
    // exit gracefully after cleanup when no process is required
    exit(EXIT_SUCCESS);
  }
  catch (const std::exception& ex)
  {
    f3d::log::error("F3D encountered an unexpected exception:");
    f3d::log::error(ex.what());
    exit(EXIT_FAILURE);
  }

  return res;
}

#ifdef _WIN32
#include <Windows.h>
#include <clocale>
#include <codecvt>
#include <locale>
#include <string>

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
