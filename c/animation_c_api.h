#ifndef F3D_ANIMATION_C_API_H
#define F3D_ANIMATION_C_API_H

#include "export.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Opaque handle to an f3d::animation object.
   */
  typedef struct f3d_animation_t f3d_animation_t;

  /**
   * @brief Enumeration of animation direction.
   */
  typedef enum f3d_animation_direction_t
  {
    F3D_ANIMATION_FORWARD,
    F3D_ANIMATION_BACKWARD
  } f3d_animation_direction_t;

  /**
   * @brief Load files in the scene at provided time value if they contain any animation.
   *
   * @param animation Animation handle.
   * @param time_value Time value to load.
   */
  F3D_EXPORT void f3d_animation_load_time(f3d_animation_t* animation, double time_value);

  /**
   * @brief Get key frames times of files in the scene
   *
   * The returned key frames is heap-allocated and must be freed with
   * f3d_animation_destroy_key_frames().
   *
   * @param animation Animation handle.
   * @param count Pointer to store the count of key frames
   * @return Pointer to the array of key frame time keys
   */
  F3D_EXPORT double* f3d_animation_get_key_frames(f3d_animation_t* animation, unsigned int* count);

  /**
   * @brief Free the animation key frames array.
   *
   * @param key_frames Pointer to the key_frames array to free.
   */
  F3D_EXPORT void f3d_animation_destroy_key_frames(double* key_frames);

  /**
   * @brief Get animation time range of current files in the scene.
   *
   * @param animation Animation handle.
   * @param min_time Pointer to store minimum time.
   * @param max_time Pointer to store maximum time.
   */
  F3D_EXPORT void f3d_animation_get_time_range(
    f3d_animation_t* animation, double* min_time, double* max_time);

  /**
   * @brief Return the number of animations available in the current files in the scene.
   *
   * @param animation Animation handle.
   * @return Number of available animations.
   */
  F3D_EXPORT unsigned int f3d_animation_count(const f3d_animation_t* animation);

  /**
   * @brief Get the current animation name.
   *
   * The returned string must be freed with f3d_animation_destroy_string().
   *
   * @param animation Animation handle.
   * @return animation name, or NULL on failure.
   */
  F3D_EXPORT char* f3d_animation_get_current_name(const f3d_animation_t* animation);

  /**
   * @brief Get the animation name at provided index.
   *
   * The returned string must be freed with f3d_animation_destroy_string().
   *
   * @param animation Animation handle.
   * @param index Index of animation.
   * @return animation name, or NULL on failure.
   */
  F3D_EXPORT char* f3d_animation_get_name(const f3d_animation_t* animation, int index);

  /**
   * @brief Get all animation names
   *
   * @param animation animation handle.
   * @param count Output parameter for number of actions.
   * @return Array of action strings. Caller must free the array with
   *         f3d_animation_destroy_string_array().
   */
  F3D_EXPORT char** f3d_animation_get_names(const f3d_animation_t* animation, int* count);

  /**
   * @brief Toggle the animation.
   *
   * @param animation Animation handle.
   * @param direction Animation direction.
   */
  F3D_EXPORT void f3d_animation_toggle(
    f3d_animation_t* animation, f3d_animation_direction_t direction);

  /**
   * @brief Start the animation.
   *
   * @param animation Animation handle.
   * @param direction Animation direction.
   */
  F3D_EXPORT void f3d_animation_start(
    f3d_animation_t* animation, f3d_animation_direction_t direction);

  /**
   * @brief Stop the animation.
   *
   * @param animation Animation handle.
   */
  F3D_EXPORT void f3d_animation_stop(f3d_animation_t* animation);

  /**
   * @brief Check if animation is currently playing.
   *
   * @param animation Animation handle.
   * @return 1 if animation is playing, 0 otherwise.
   */
  F3D_EXPORT int f3d_animation_playing(f3d_animation_t* animation);

  /**
   * @brief Get the current animation direction.
   *
   * @param animation Animation handle.
   * @return Current animation direction.
   */
  F3D_EXPORT f3d_animation_direction_t f3d_animation_direction(
    f3d_animation_t* animation);
  ///@}

  /**
   * @brief Free a single string returned by the animation C API.
   *
   * @param str String to free.
   */
  F3D_EXPORT void f3d_animation_destroy_string(const char* str);

  /**
   * @brief Free a string array returned by animation functions.
   *
   * @param array String array to free.
   * @param count Number of strings in the array.
   */
  F3D_EXPORT void f3d_animation_destroy_string_array(char** array, int count);

#ifdef __cplusplus
}
#endif

#endif // F3D_ANIMATION_C_API_H
