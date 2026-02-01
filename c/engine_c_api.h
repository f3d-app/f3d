#ifndef F3D_ENGINE_C_API_H
#define F3D_ENGINE_C_API_H

#include "context_c_api.h"
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

  /**
   * @brief Structure representing a rendering backend with its availability.
   */
  typedef struct
  {
    char* name;    /**< Backend name (e.g., "GLX", "EGL", "WGL", etc.) */
    int available; /**< Non-zero if backend is available, zero otherwise */
  } f3d_backend_info_t;

  /**
   * @brief Structure representing a module with its availability.
   */
  typedef struct
  {
    char* name;    /**< Module name */
    int available; /**< Non-zero if module is available, zero otherwise */
  } f3d_module_info_t;

  /**
   * @brief Structure providing information about the libf3d.
   */
  typedef struct
  {
    char* version;              /**< Version string */
    char* version_full;         /**< Full version string */
    char* build_date;           /**< Build date */
    char* build_system;         /**< Build system */
    char* compiler;             /**< Compiler used */
    f3d_module_info_t* modules; /**< NULL-terminated array of modules */
    char* vtk_version;          /**< VTK version */
    char** copyrights;          /**< NULL-terminated array of copyright strings */
    char* license;              /**< License text */
  } f3d_lib_info_t;

  /**
   * @brief Structure providing information about a reader.
   */
  typedef struct
  {
    char* name;              /**< Reader name */
    char* description;       /**< Reader description */
    char** extensions;       /**< NULL-terminated array of file extensions */
    char** mime_types;       /**< NULL-terminated array of MIME types */
    char* plugin_name;       /**< Plugin name */
    int has_scene_reader;    /**< Non-zero if has scene reader */
    int has_geometry_reader; /**< Non-zero if has geometry reader */
  } f3d_reader_info_t;

  ///@{ @name Engine factory methods
  /**
   * @brief Create an engine with an automatic window.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @param offscreen If non-zero, the window will be hidden.
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create(int offscreen);

  /**
   * @brief Create an engine with no window.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_none();

  /**
   * @brief Create an engine with a GLX window (Linux only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @param offscreen If non-zero, the window will be hidden.
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_glx(int offscreen);

  /**
   * @brief Create an engine with a WGL window (Windows only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @param offscreen If non-zero, the window will be hidden.
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_wgl(int offscreen);

  /**
   * @brief Create an engine with an offscreen EGL window.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_egl();

  /**
   * @brief Create an engine with an offscreen OSMesa window.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_osmesa();

  /**
   * @brief Create an engine with an external window.
   *
   * A context to retrieve OpenGL symbols is required.
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @param get_proc_address Function pointer for OpenGL symbol resolution.
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external(f3d_context_function_t get_proc_address);

  /**
   * @brief Create an engine with an external GLX context (Linux only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external_glx();

  /**
   * @brief Create an engine with an external WGL context (Windows only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external_wgl();

  /**
   * @brief Create an engine with an external COCOA context (macOS only).
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external_cocoa();

  /**
   * @brief Create an engine with an external EGL context.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle, NULL on failure.
   */
  F3D_EXPORT f3d_engine_t* f3d_engine_create_external_egl();

  /**
   * @brief Create an engine with an external OSMesa context.
   *
   * The returned engine must be deleted with f3d_engine_delete().
   *
   * @return Engine handle, NULL on failure.
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
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_engine_set_cache_path(f3d_engine_t* engine, const char* cache_path);

  /**
   * @brief Set options for the engine.
   *
   * This will copy the provided options into the engine.
   *
   * @param engine Engine handle.
   * @param options Options handle to copy from.
   */
  F3D_EXPORT void f3d_engine_set_options(f3d_engine_t* engine, f3d_options_t* options);

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

  ///@{ @name Rendering backends
  /**
   * @brief List rendering backends supported by libf3d.
   *
   * Returns a map of backend names with boolean flags indicating availability.
   * The returned array of key-value pairs is NULL-terminated and must be freed
   * by the caller using f3d_engine_free_backend_list().
   *
   * @param count Pointer to store the number of backends (optional, can be NULL).
   * @return NULL-terminated array of backend name/availability pairs.
   */
  F3D_EXPORT f3d_backend_info_t* f3d_engine_get_rendering_backend_list(int* count);

  ///@{ @name Plugin management
  /**
   * @brief Load a plugin.
   *
   * @param path_or_name Plugin path or name.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_engine_load_plugin(const char* path_or_name);

  /**
   * @brief Automatically load all static plugins.
   */
  F3D_EXPORT void f3d_engine_autoload_plugins();

  /**
   * @brief List plugins based on associated json files located in the given directory.
   *
   * Listed plugins can be loaded using f3d_engine_load_plugin function.
   * The returned array is NULL-terminated and must be freed by the caller using
   * f3d_engine_free_string_array().
   *
   * @param plugin_path Path to the directory containing plugin json files.
   * @return NULL-terminated array of plugin name strings, or NULL if the directory doesn't exist.
   */
  F3D_EXPORT char** f3d_engine_get_plugins_list(const char* plugin_path);
  ///@}

  ///@{ @name Reader options
  /**
   * @brief Get all plugin option names that can be set using f3d_engine_set_reader_option.
   *
   * This vector can be expanded when loading plugins using f3d_engine_load_plugin.
   * The returned array is NULL-terminated and must be freed by the caller using
   * f3d_engine_free_string_array().
   *
   * @return NULL-terminated array of option name strings.
   */
  F3D_EXPORT char** f3d_engine_get_all_reader_option_names();

  /**
   * @brief Set a specific reader option.
   *
   * @param name Option name.
   * @param value Option value.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_engine_set_reader_option(const char* name, const char* value);
  ///@}

  /**
   * @brief Free a backend list returned by f3d_engine_get_rendering_backend_list().
   *
   * @param backends Backend list to free.
   */
  F3D_EXPORT void f3d_engine_free_backend_list(f3d_backend_info_t* backends);
  ///@}

  ///@{ @name Library information
  /**
   * @brief Get information about the libf3d.
   *
   * The returned structure must be freed by the caller using f3d_engine_free_lib_info().
   *
   * @return Library information structure.
   */
  F3D_EXPORT f3d_lib_info_t* f3d_engine_get_lib_info();

  /**
   * @brief Free a lib info structure returned by f3d_engine_get_lib_info().
   *
   * @param info Lib info structure to free.
   */
  F3D_EXPORT void f3d_engine_free_lib_info(f3d_lib_info_t* info);

  /**
   * @brief Get information about the supported readers.
   *
   * The returned array is NULL-terminated and must be freed by the caller using
   * f3d_engine_free_readers_info().
   *
   * @param count Pointer to store the number of readers (optional, can be NULL).
   * @return NULL-terminated array of reader information structures.
   */
  F3D_EXPORT f3d_reader_info_t* f3d_engine_get_readers_info(int* count);

  /**
   * @brief Free a readers info array returned by f3d_engine_get_readers_info().
   *
   * @param readers Readers info array to free.
   */
  F3D_EXPORT void f3d_engine_free_readers_info(f3d_reader_info_t* readers);
  ///@}

  ///@{ @name Utility functions
  /**
   * @brief Free a NULL-terminated string array.
   *
   * @param array String array to free.
   */
  F3D_EXPORT void f3d_engine_free_string_array(char** array);
  ///@}

#ifdef __cplusplus
}
#endif

#endif // F3D_ENGINE_C_API_H
