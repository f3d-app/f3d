#include "F3DSystemTools.h"

#include "log.h"

#include <array>
#include <cstring>
#include <filesystem>

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
  std::array<wchar_t, 1024> buffer{};
  if (GetModuleFileNameW(nullptr, buffer.data(), 1024))
  {
    return fs::path(buffer.data());
  }
  f3d::log::error("Cannot retrieve application path");
  return fs::path();
#else
#ifdef __APPLE__
  uint32_t size = 1024;
  std::array<char, 1024> buffer;
  if (_NSGetExecutablePath(buffer.data(), &size) != 0)
  {
    f3d::log::error("Cannot retrieve application path");
    return fs::path();
  }
  return fs::path(buffer.data());
#else
  try
  {
    return fs::canonical("/proc/self/exe");
  }
  catch (const std::exception& ex)
  {
    f3d::log::error("Cannot retrieve application path: ", ex.what());
    return fs::path();
  }
#endif
#endif
}
}
