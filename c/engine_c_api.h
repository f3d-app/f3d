#ifndef F3D_ENGINE_C_API_H
#define F3D_ENGINE_C_API_H

#include "export.h"
#include "interactor_c_api.h"
#include "scene_c_api.h"
#include "window_c_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Opaque handle to an f3d::engine object.
   */
  typedef struct f3d_engine_t f3d_engine_t;

  /**
   * @brief Opaque handle to f3d::options object (managed by engine).
   */
  typedef struct f3d_options_t f3d_options_t;

  ///@{ @name Engine factory methods
  /**
   * @brief Create an engine with an automatic window.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @param offscreen If non-zero, the window will be hidden.
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create(int offscreen);

  /**
   * @brief Create an engine with no window.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_none();

  /**
   * @brief Create an engine with a GLX window (Linux only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @param offscreen If non-zero, the window will be hidden.
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_glx(int offscreen);

  /**
   * @brief Create an engine with a WGL window (Windows only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @param offscreen If non-zero, the window will be hidden.
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_wgl(int offscreen);

  /**
   * @brief Create an engine with an offscreen EGL window.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_egl();

  /**
   * @brief Create an engine with an offscreen OSMesa window.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_osmesa();

  /**
   * @brief Create an engine with an external GLX context (Linux only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external_glx();

  /**
   * @brief Create an engine with an external WGL context (Windows only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external_wgl();

  /**
   * @brief Create an engine with an external COCOA context (macOS only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external_cocoa();

  /**
   * @brief Create an engine with an external EGL context.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external_egl();

  /**
   * @brief Create an engine with an external OSMesa context.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external_osmesa();
  ///@}

  /**
   * @brief Destroy an engine and free associated resources.
   *
   * @param engine Engine handle.
   */
  F3D_EXPORT void f3d_engine_delete(f3d_engine_t* engine);

  /**
   * @brief Set the cache path directory.
   *
   * @param engine Engine handle.
   * @param cache_path Cache path string.
   */
  F3D_EXPORT void f3d_engine_set_cache_path(f3d_engine_t* engine, const char* cache_path);

  /**
   * @brief Get the options object from the engine.
   *
   * @param engine Engine handle.
   * @return Options handle (not owned by caller, managed by engine).
   */
  F3D_EXPORT f3d_options_t* f3d_engine_get_options(f3d_engine_t* engine);

  /**
   * @brief Get the window from the engine.
   *
   * @param engine Engine handle.
   * @return Window handle (not owned by caller).
   */
  F3D_EXPORT f3d_window_t* f3d_engine_get_window(f3d_engine_t* engine);

  /**
   * @brief Get the scene from the engine.
   *
   * @param engine Engine handle.
   * @return Scene handle (not owned by caller).
   */
  F3D_EXPORT f3d_scene_t* f3d_engine_get_scene(f3d_engine_t* engine);

  /**
   * @brief Get the interactor from the engine.
   *
   * @param engine Engine handle.
   * @return Interactor handle (not owned by caller).
   */
  F3D_EXPORT f3d_interactor_t* f3d_engine_get_interactor(f3d_engine_t* engine);

  ///@{ @name Plugin management
  /**
   * @brief Load a plugin.
   *
   * @param path_or_name Plugin path or name.
   */
  F3D_EXPORT void f3d_engine_load_plugin(const char* path_or_name);

  /**
   * @brief Automatically load all static plugins.
   */
  F3D_EXPORT void f3d_engine_autoload_plugins();
  ///@}

  ///@{ @name Reader options
  /**
   * @brief Set a specific reader option.
   *
   * @param name Option name.
   * @param value Option value.
   */
  F3D_EXPORT void f3d_engine_set_reader_option(const char* name, const char* value);
  ///@}

#ifdef __cplusplus
}
#endif

#endif // F3D_ENGINE_C_API_H
