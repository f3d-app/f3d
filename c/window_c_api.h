#ifndef F3D_WINDOW_C_API_H
#define F3D_WINDOW_C_API_H

#include "camera_c_api.h"
#include "export.h"
#include "types_c_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Opaque handle to an f3d::window object.
   */
  typedef struct f3d_window_t f3d_window_t;

  /**
   * @brief Enumeration of supported window types.
   */
  typedef enum f3d_window_type_t
  {
    F3D_WINDOW_NONE,
    F3D_WINDOW_EXTERNAL,
    F3D_WINDOW_GLX,
    F3D_WINDOW_WGL,
    F3D_WINDOW_COCOA,
    F3D_WINDOW_EGL,
    F3D_WINDOW_OSMESA,
    F3D_WINDOW_WASM,
    F3D_WINDOW_UNKNOWN
  } f3d_window_type_t;

  /**
   * @brief Get the type of the window.
   *
   * @param window Window handle.
   * @return The window type.
   */
  F3D_EXPORT f3d_window_type_t f3d_window_get_type(f3d_window_t* window);

  /**
   * @brief Check if the window is offscreen.
   *
   * @param window Window handle.
   * @return 1 if offscreen, 0 otherwise.
   */
  F3D_EXPORT int f3d_window_is_offscreen(f3d_window_t* window);

  /**
   * @brief Get the camera provided by the window.
   *
   * @param window Window handle.
   * @return Camera handle.
   */
  F3D_EXPORT f3d_camera_t* f3d_window_get_camera(f3d_window_t* window);

  /**
   * @brief Perform a render of the window to the screen.
   *
   * All dynamic options are updated if needed.
   *
   * @param window Window handle.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_window_render(f3d_window_t* window);

  /**
   * @brief Set the size of the window.
   *
   * @param window Window handle.
   * @param width Window width in pixels.
   * @param height Window height in pixels.
   */
  F3D_EXPORT void f3d_window_set_size(f3d_window_t* window, int width, int height);

  /**
   * @brief Get the width of the window.
   *
   * @param window Window handle.
   * @return Window width in pixels.
   */
  F3D_EXPORT int f3d_window_get_width(const f3d_window_t* window);

  /**
   * @brief Get the height of the window.
   *
   * @param window Window handle.
   * @return Window height in pixels.
   */
  F3D_EXPORT int f3d_window_get_height(const f3d_window_t* window);

  /**
   * @brief Set the position of the window.
   *
   * @param window Window handle.
   * @param x X position in pixels.
   * @param y Y position in pixels.
   */
  F3D_EXPORT void f3d_window_set_position(f3d_window_t* window, int x, int y);

  /**
   * @brief Set the icon to be shown by a window manager.
   *
   * @param window Window handle.
   * @param icon Icon data as unsigned char array.
   * @param icon_size Size of icon data in bytes.
   */
  F3D_EXPORT void f3d_window_set_icon(
    f3d_window_t* window, const unsigned char* icon, size_t icon_size);

  /**
   * @brief Set the window name to be shown by a window manager.
   *
   * @param window Window handle.
   * @param window_name Window name string.
   */
  F3D_EXPORT void f3d_window_set_window_name(f3d_window_t* window, const char* window_name);

  /**
   * @brief Convert a point in display coordinate to world coordinate.
   *
   * @param window Window handle.
   * @param display_point Display coordinate point [x, y, z].
   * @param world_point Output world coordinate point [x, y, z].
   */
  F3D_EXPORT void f3d_window_get_world_from_display(
    const f3d_window_t* window, const f3d_point3_t display_point, f3d_point3_t world_point);

  /**
   * @brief Convert a point in world coordinate to display coordinate.
   *
   * @param window Window handle.
   * @param world_point World coordinate point [x, y, z].
   * @param display_point Output display coordinate point [x, y, z].
   */
  F3D_EXPORT void f3d_window_get_display_from_world(
    const f3d_window_t* window, const f3d_point3_t world_point, f3d_point3_t display_point);

#ifdef __cplusplus
}
#endif

#endif // F3D_WINDOW_C_API_H
