#ifndef F3D_CONTEXT_C_API_H
#define F3D_CONTEXT_C_API_H

#include "export.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Function pointer type for OpenGL symbol resolution.
   */
  typedef void (*(*f3d_context_function_t)(const char*))();

  /**
   * @brief Opaque handle to a context object.
   */
  typedef struct f3d_context f3d_context_t;

  /**
   * @brief Create a GLX context.
   *
   * The returned context must be deleted with f3d_context_delete().
   *
   * @return Context handle.
   */
  F3D_EXPORT f3d_context_t* f3d_context_glx();

  /**
   * @brief Create a WGL context.
   *
   * The returned context must be deleted with f3d_context_delete().
   *
   * @return Context handle.
   */
  F3D_EXPORT f3d_context_t* f3d_context_wgl();

  /**
   * @brief Create a COCOA context.
   *
   * The returned context must be deleted with f3d_context_delete().
   *
   * @return Context handle.
   */
  F3D_EXPORT f3d_context_t* f3d_context_cocoa();

  /**
   * @brief Create an EGL context.
   *
   * The returned context must be deleted with f3d_context_delete().
   *
   * @return Context handle.
   */
  F3D_EXPORT f3d_context_t* f3d_context_egl();

  /**
   * @brief Create an OSMesa context.
   *
   * The returned context must be deleted with f3d_context_delete().
   *
   * @return Context handle.
   */
  F3D_EXPORT f3d_context_t* f3d_context_osmesa();

  /**
   * @brief Create a context from a library name and function name.
   *
   * The returned context must be deleted with f3d_context_delete().
   *
   * @param lib Library name.
   * @param func Function name to resolve.
   * @return Context handle.
   */
  F3D_EXPORT f3d_context_t* f3d_context_get_symbol(const char* lib, const char* func);

  /**
   * @brief Delete a context object.
   *
   * @param ctx Context handle.
   */
  F3D_EXPORT void f3d_context_delete(f3d_context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // F3D_CONTEXT_C_API_H
