#ifndef f3d_context_h
#define f3d_context_h

#include "exception.h"
#include "export.h"

#include <functional>
#include <string>

namespace f3d
{
/**
 * @class   context
 * @brief   Utility function that implements basic OpenGL context symbol resolvers
 *
 * These functions should be used only when the rendering context (the OpenGL window) isn't
 * managed by libf3d, but by the user directly.
 * Therefore, the engine is created using `f3d::engine::createExternal` and the functions below
 * can be used as argument of this factory.
 * One can also create a custom functor returning a `f3d::context::function`.
 */
class F3D_EXPORT context
{
public:
  using fptr = void (*)();
  using function = std::function<fptr(const char*)>;

  /**
   * Create a GLX context function.
   * Only supported on Linux.
   * Throw a loading_exception if it fails.
   */
  [[nodiscard]] static function glx();

  /**
   * Create a WGL context function.
   * Only supported on Windows.
   * Throw a loading_exception if it fails.
   */
  [[nodiscard]] static function wgl();

  /**
   * Create a COCOA context function.
   * This is usually required when using a headless context and a GPU device.
   * Only supported on macOS.
   * Throw a loading_exception if it fails.
   */
  [[nodiscard]] static function cocoa();

  /**
   * Create a EGL context function.
   * This is usually required when using a headless context and a GPU device.
   * Only supported on Linux and Windows.
   * Throw a loading_exception if it fails.
   */
  [[nodiscard]] static function egl();

  /**
   * Create a OSMesa context function.
   * This is usually required when using a headless context and no GPU device.
   * Only supported on Linux and Windows.
   * Throw a loading_exception if it fails.
   */
  [[nodiscard]] static function osmesa();

  /**
   * Create a context function from a library name and a function name.
   * The library name must be specified without its prefix and extension.
   * For example, `getSymbol("EGL", "eglGetProcAddress")` looks for the symbol
   * `eglGetProcAddress` in the library `libEGL.so` on Linux.
   * Throw a loading_exception if it fails to find the library or a symbol_exception
   * if the lib does not contains the expected symbol.
   */
  [[nodiscard]] static function getSymbol(std::string_view lib, std::string_view func);

  /**
   * An exception that can be thrown when the requested library cannot be loaded.
   */
  struct loading_exception : public exception
  {
    explicit loading_exception(const std::string& what = "");
  };

  /**
   * An exception that can be thrown when the symbol resolver cannot be found in the library.
   */
  struct symbol_exception : public exception
  {
    explicit symbol_exception(const std::string& what = "");
  };
};
}

#endif
