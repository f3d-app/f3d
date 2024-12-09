#include "context.h"

#ifdef __APPLE__
#include "context_cocoa.h"
#endif

#include <vtkRenderingOpenGLConfigure.h>
#include <vtkVersion.h>

#if defined(VTK_OPENGL_HAS_EGL) && VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
#include <vtkglad/include/glad/egl.h>
#endif

#include <vtksys/DynamicLoader.hxx>

namespace f3d
{
//----------------------------------------------------------------------------
context::function context::getSymbol(const std::string& lib, const std::string& func)
{
  std::string libName = vtksys::DynamicLoader::LibPrefix();
  libName += lib;
  libName += vtksys::DynamicLoader::LibExtension();

  vtksys::DynamicLoader::LibraryHandle handle = vtksys::DynamicLoader::OpenLibrary(libName);

  if (!handle)
  {
    throw context::loading_exception("Cannot find " + lib + " library");
  }

  using symbol = context::fptr (*)(const char*);

  symbol address = reinterpret_cast<symbol>(vtksys::DynamicLoader::GetSymbolAddress(handle, func));

  if (!address)
  {
    throw context::symbol_exception("Cannot find " + func + " symbol");
  }

  return address;
}

//----------------------------------------------------------------------------
context::function context::glx()
{
#if defined(VTK_USE_X) && VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
  return getSymbol("GLX", "glXGetProcAddress");
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
#if defined(VTK_OPENGL_HAS_EGL) && VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240914)
  gladLoaderLoadEGL(EGL_NO_DISPLAY);
  return getSymbol("EGL", "eglGetProcAddress");
#else
  throw loading_exception("Cannot use a EGL context on this platform");
#endif
}

//----------------------------------------------------------------------------
context::function context::osmesa()
{
#ifdef __linux__
  return getSymbol("OSMesa", "OSMesaGetProcAddress");
#elif _WIN32
  return getSymbol("osmesa", "OSMesaGetProcAddress");
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
