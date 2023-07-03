#include "F3DSystemTools.h"

#include "log.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace fs = std::filesystem;

namespace F3DSystemTools
{
//----------------------------------------------------------------------------
fs::path GetApplicationPath()
{
#if defined(_WIN32)
  std::array<wchar_t, 1024> wc{};
  if (GetModuleFileNameW(nullptr, wc.data(), 1024))
  {
    return fs::path(wc.data());
  }
  f3d::log::error("Cannot retrieve application path");
  return {};
#else
#ifdef __APPLE__
  uint32_t size = 1024;
  std::array<char, 1024> buffer;
  if (_NSGetExecutablePath(buffer.data(), &size) != 0)
  {
    f3d::log::error("Executable is too long to recover application path");
    return {};
  }
  return fs::path(buffer.data());
#else
  try
  {
#if defined(__FreeBSD__)
    return fs::canonical("/proc/curproc/file");
#else
    return fs::canonical("/proc/self/exe");
#endif
  }
  catch (const std::exception& ex)
  {
    f3d::log::error("Cannot retrieve application path: ", ex.what());
    return {};
  }
#endif
#endif
}

std::vector<std::string> GetVectorEnvironnementVariable(const std::string& envVar)
{
  const char* envPtr = std::getenv(envVar.c_str());
  if (!envPtr)
  {
    return {};
  }

  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(envPtr);

  // split path with OS separator (':' on Linux/macOS and ';' on Windows)
#ifdef _WIN32
  char delimiter = ';';
#else
  char delimiter = ':';
#endif

  while (std::getline(tokenStream, token, delimiter))
  {
    tokens.push_back(token);
  }

  return tokens;
}
}
