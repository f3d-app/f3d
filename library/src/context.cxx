#include "context.h"

#ifdef __APPLE__
#include "context_cocoa.h"
#endif

#include <vtkRenderingOpenGLConfigure.h>
#include <vtkVersion.h>

#if defined(VTK_OPENGL_HAS_EGL)
#include <vtkglad/include/glad/egl.h>
#endif

#include <vtksys/DynamicLoader.hxx>

namespace f3d
{
//----------------------------------------------------------------------------
context::function context::getSymbol(std::string_view lib, std::string_view func)
{
#ifdef _WIN32
  // on Windows vtksys::DynamicLoader::OpenLibrary behaves differently (it expects a full path)
  vtksys::DynamicLoader::LibraryHandle handle = LoadLibraryA(lib.data());
#else
  vtksys::DynamicLoader::LibraryHandle handle = vtksys::DynamicLoader::OpenLibrary(lib.data());
#endif

  if (!handle)
  {
    throw context::loading_exception("Cannot find " + std::string(lib) + " library");
  }

  using symbol = context::fptr (*)(const char*);

  symbol address =
    reinterpret_cast<symbol>(vtksys::DynamicLoader::GetSymbolAddress(handle, func.data()));

  if (!address)
  {
    throw context::symbol_exception("Cannot find " + std::string(func) + " symbol");
  }

  return address;
}

//----------------------------------------------------------------------------
context::function context::glx()
{
#if defined(VTK_USE_X)
  return getSymbol("libGLX.so", "glXGetProcAddress");
#else
  throw loading_exception("Cannot use a GLX context on this platform");
#endif
}

//----------------------------------------------------------------------------
context::function context::wgl()
{
#ifdef _WIN32
  return [](const char* name)
  {
    fptr p = reinterpret_cast<fptr>(wglGetProcAddress(name));
    if (p == nullptr)
    {
      HMODULE module = LoadLibraryA("opengl32.dll");
      p = reinterpret_cast<fptr>(GetProcAddress(module, name));
    }

    return p;
  };
#else
  throw loading_exception("Cannot use a WGL context on this platform");
#endif
}

//----------------------------------------------------------------------------
context::function context::cocoa()
{
#ifdef __APPLE__
  return [](const char* name)
  {
    fptr p = reinterpret_cast<fptr>(detail::getCocoaOpenGLSymbol(name));
    return p;
  };
#else
  throw loading_exception("Cannot use a COCOA context on this platform");
#endif
}

//----------------------------------------------------------------------------
context::function context::egl()
{
#if defined(VTK_OPENGL_HAS_EGL)
  gladLoaderLoadEGL(EGL_NO_DISPLAY);

#ifdef _WIN32
  for (const auto& lib : { "libEGL.dll", "EGL.dll" })
#else
  for (const auto& lib : { "libEGL.so.1", "libEGL.so" })
#endif
  {
    try
    {
      return getSymbol(lib, "eglGetProcAddress");
    }
    catch (const loading_exception&)
    {
    }
  }
  throw loading_exception("Cannot find EGL library");
#else
  throw loading_exception("Cannot use a EGL context on this platform");
#endif
}

//----------------------------------------------------------------------------
context::function context::osmesa()
{
#if (defined(__APPLE__) && VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 7, 20260706)) ||             \
  defined(__linux__) || defined(__FreeBSD__) || defined(_WIN32)
#ifdef _WIN32
  for (const auto& lib : { "osmesa.dll" })
#elif __APPLE__
  for (const auto& lib : { "libOSMesa.8.dylib", "libOSMesa.6.dylib", "libOSMesa.dylib" })
#else
  for (const auto& lib : { "libOSMesa.so.8", "libOSMesa.so.6", "libOSMesa.so" })
#endif
  {
    try
    {
      return getSymbol(lib, "OSMesaGetProcAddress");
    }
    catch (const loading_exception&)
    {
    }
  }
  throw loading_exception("Cannot find OSMesa library");
#else
  throw loading_exception("Cannot use a OSMesa context on this platform");
#endif
}

//----------------------------------------------------------------------------
context::loading_exception::loading_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
context::symbol_exception::symbol_exception(const std::string& what)
  : exception(what)
{
}
}
