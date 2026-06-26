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
std::string context::addLibraryDecoration(std::string_view lib)
{
  std::string libName = vtksys::DynamicLoader::LibPrefix();
  libName += lib;
  libName += vtksys::DynamicLoader::LibExtension();
  return libName;
}

//----------------------------------------------------------------------------
context::function context::getSymbol(std::string_view lib, std::string_view func)
{
  vtksys::DynamicLoader::LibraryHandle handle = vtksys::DynamicLoader::OpenLibrary(lib.data());

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
  return getSymbol(addLibraryDecoration("GLX"), "glXGetProcAddress");
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
  return getSymbol(addLibraryDecoration("EGL"), "eglGetProcAddress");
#else
  throw loading_exception("Cannot use a EGL context on this platform");
#endif
}

//----------------------------------------------------------------------------
context::function context::osmesa()
{
#if defined(__APPLE__)
  return getSymbol("libOSMesa.dylib", "OSMesaGetProcAddress");
#elif defined(__linux__) || defined(__FreeBSD__)
  return getSymbol("libOSMesa.so", "OSMesaGetProcAddress");
#elif _WIN32
  return getSymbol("osmesa.dll", "OSMesaGetProcAddress");
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
