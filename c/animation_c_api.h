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
   * @brief Load files in the scene at provided time value if they contain any animation.
   *
   * @param animation Animation handle.
   * @param time_value Time value to load.
   */
  F3D_EXPORT void f3d_animation_load_time(f3d_animation_t* animation, double time_value);

  /**
   * @brief Get keyframes times of files in the scene
   *
   * The returned keyframes is heap-allocated and must be freed with
   * f3d_animation_destroy_keyframes().
   *
   * @param animation Animation handle.
   * @param count Pointer to store the count of keyframes
   * @return Pointer to the array of keyframe time keys
   */
  F3D_EXPORT double* f3d_animation_keyframes(f3d_animation_t* animation, unsigned int* count);

  /**
   * @brief Free the animation keyframes array.
   *
   * @param keyframes Pointer to the keyframes array to free.
   */
  F3D_EXPORT void f3d_animation_destroy_keyframes(double* keyframes);

  /**
   * @brief Get animation time range of current files in the scene.
   *
   * @param animation Animation handle.
   * @param min_time Pointer to store minimum time.
   * @param max_time Pointer to store maximum time.
   */
  F3D_EXPORT void f3d_animation_time_range(
    f3d_animation_t* animation, double* min_time, double* max_time);

  /**
   * @brief Return the number of animations available in the current files in the scene.
   *
   * @param animation Animation handle.
   * @return Number of available animations.
   */
  F3D_EXPORT unsigned int f3d_animation_count(const f3d_animation_t* animation);

#ifdef __cplusplus
}
#endif

#endif // F3D_ANIMATION_C_API_H
