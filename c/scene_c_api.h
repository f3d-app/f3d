#ifndef F3D_SCENE_C_API_H
#define F3D_SCENE_C_API_H

#include "export.h"
#include "types_c_api.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Opaque handle to an f3d::scene object.
   */
  typedef struct f3d_scene_t f3d_scene_t;

  /**
   * @brief Add and load a file into the scene.
   *
   * @param scene Scene handle.
   * @param file_path File path to add.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_scene_add(f3d_scene_t* scene, const char* file_path);

  /**
   * @brief Add and load multiple files into the scene.
   *
   * @param scene Scene handle.
   * @param file_paths Array of file paths.
   * @param count Number of file paths in the array.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_scene_add_multiple(f3d_scene_t* scene, const char** file_paths, size_t count);

  /**
   * @brief Add and load a mesh into the scene.
   *
   * @param scene Scene handle.
   * @param mesh Mesh structure.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_scene_add_mesh(f3d_scene_t* scene, const f3d_mesh_t* mesh);

  /**
   * @brief Add and load a memory buffer into the scene.
   *
   * @param scene Scene handle.
   * @param buffer Memory buffer containing a file.
   * @param size Size of the buffer in bytes.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_scene_add_buffer(f3d_scene_t* scene, void* buffer, size_t size);

  /**
   * @brief Clear the scene of all added files.
   *
   * @param scene Scene handle.
   */
  F3D_EXPORT void f3d_scene_clear(f3d_scene_t* scene);

  /**
   * @brief Add a light based on a light state.
   *
   * @param scene Scene handle.
   * @param light_state Light state structure.
   * @return Index of the added light.
   */
  F3D_EXPORT int f3d_scene_add_light(
    const f3d_scene_t* scene, const f3d_light_state_t* light_state);

  /**
   * @brief Get the number of lights.
   *
   * @param scene Scene handle.
   * @return Number of lights in the scene.
   */
  F3D_EXPORT int f3d_scene_get_light_count(const f3d_scene_t* scene);

  /**
   * @brief Get the light state at provided index.
   *
   * The returned light_state is heap-allocated and must be freed with
   * f3d_light_state_free().
   *
   * @param scene Scene handle.
   * @param index Index of the light.
   * @return Light state, NULL on failure.
   */
  F3D_EXPORT f3d_light_state_t* f3d_scene_get_light(const f3d_scene_t* scene, int index);

  /**
   * @brief Update a light at provided index with the provided light state.
   *
   * @param scene Scene handle.
   * @param index Index of the light to update.
   * @param light_state New light state.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_scene_update_light(
    f3d_scene_t* scene, int index, const f3d_light_state_t* light_state);

  /**
   * @brief Remove a light at provided index.
   *
   * @param scene Scene handle.
   * @param index Index of the light to remove.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_scene_remove_light(f3d_scene_t* scene, int index);

  /**
   * @brief Remove all lights from the scene.
   *
   * @param scene Scene handle.
   */
  F3D_EXPORT void f3d_scene_remove_all_lights(f3d_scene_t* scene);

  /**
   * @brief Check if a file path is supported by the scene.
   *
   * @param scene Scene handle.
   * @param file_path File path to check.
   * @return 1 if supported, 0 otherwise.
   */
  F3D_EXPORT int f3d_scene_supports(f3d_scene_t* scene, const char* file_path);

  /**
   * @brief Load added files at provided time value if they contain any animation.
   *
   * @param scene Scene handle.
   * @param time_value Time value to load.
   */
  F3D_EXPORT void f3d_scene_load_animation_time(f3d_scene_t* scene, double time_value);

  /**
   * @brief Get keyframes times of loaded files
   *
   * The returned time must be freed with f3d_scene_free_animation_keyframes.
   *
   * @param scene Scene handle.
   * @param count Pointer to store the count of keyframes
   * @return Pointer to the array of keyframe time keys
   */
  F3D_EXPORT double* f3d_scene_get_animation_keyframes(f3d_scene_t* scene, unsigned int* count);

  /**
   * @brief Get animation time range of currently added files.
   *
   * @param scene Scene handle.
   * @param min_time Pointer to store minimum time.
   * @param max_time Pointer to store maximum time.
   */
  F3D_EXPORT void f3d_scene_animation_time_range(
    f3d_scene_t* scene, double* min_time, double* max_time);

  /**
   * @brief Return the number of animations available in the currently loaded files.
   *
   * @param scene Scene handle.
   * @return Number of available animations.
   */
  F3D_EXPORT unsigned int f3d_scene_available_animations(const f3d_scene_t* scene);

#ifdef __cplusplus
}
#endif

#endif // F3D_SCENE_C_API_H
