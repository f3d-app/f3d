#ifndef F3D_TYPES_C_API_H
#define F3D_TYPES_C_API_H

#include "export.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Describe a 3D point.
   */
  typedef double f3d_point3_t[3];

  /**
   * @brief Describe a 3D vector.
   */
  typedef double f3d_vector3_t[3];

  /**
   * @brief Describe an angle in degrees.
   */
  typedef double f3d_angle_deg_t;

  /**
   * @brief Describe a ratio.
   */
  typedef struct f3d_ratio_t
  {
    double value;
  } f3d_ratio_t;

  /**
   * @brief Describe a RGB color.
   */
  typedef struct f3d_color_t
  {
    double data[3];
  } f3d_color_t;

  /**
   * @brief Get the red component of a color.
   */
  F3D_EXPORT double f3d_color_r(const f3d_color_t* color);

  /**
   * @brief Get the green component of a color.
   */
  F3D_EXPORT double f3d_color_g(const f3d_color_t* color);

  /**
   * @brief Get the blue component of a color.
   */
  F3D_EXPORT double f3d_color_b(const f3d_color_t* color);

  /**
   * @brief Set color components.
   */
  F3D_EXPORT void f3d_color_set(f3d_color_t* color, double r, double g, double b);

  /**
   * @brief Describe a 3D direction.
   */
  typedef struct f3d_direction_t
  {
    double data[3];
  } f3d_direction_t;

  /**
   * @brief Get the x component of a direction.
   */
  F3D_EXPORT double f3d_direction_x(const f3d_direction_t* dir);

  /**
   * @brief Get the y component of a direction.
   */
  F3D_EXPORT double f3d_direction_y(const f3d_direction_t* dir);

  /**
   * @brief Get the z component of a direction.
   */
  F3D_EXPORT double f3d_direction_z(const f3d_direction_t* dir);

  /**
   * @brief Set direction components.
   */
  F3D_EXPORT void f3d_direction_set(f3d_direction_t* dir, double x, double y, double z);

  /**
   * @brief Store a 3x3 transform matrix as a sequence of 9 double values.
   */
  typedef struct f3d_transform2d_t
  {
    double data[9];
  } f3d_transform2d_t;

  /**
   * @brief Create a 2D transform from scale, translate and angle.
   *
   * @param transform Transform structure to fill.
   * @param scale_x Scale factor in x.
   * @param scale_y Scale factor in y.
   * @param translate_x Translation in x.
   * @param translate_y Translation in y.
   * @param angle_deg Rotation angle in degrees.
   */
  F3D_EXPORT void f3d_transform2d_create(f3d_transform2d_t* transform, double scale_x,
    double scale_y, double translate_x, double translate_y, f3d_angle_deg_t angle_deg);

  /**
   * @brief Describe a colormap.
   */
  typedef struct f3d_colormap_t
  {
    double* data;
    size_t count;
  } f3d_colormap_t;

  /**
   * @brief Free a colormap structure.
   *
   * @param colormap Colormap to free.
   */
  F3D_EXPORT void f3d_colormap_free(f3d_colormap_t* colormap);

  /**
   * @brief Describe a 3D surfacic mesh.
   */
  typedef struct f3d_mesh_t
  {
    float* points;
    size_t points_count;

    float* normals;
    size_t normals_count;

    float* texture_coordinates;
    size_t texture_coordinates_count;

    unsigned int* face_sides;
    size_t face_sides_count;

    unsigned int* face_indices;
    size_t face_indices_count;
  } f3d_mesh_t;

  /**
   * @brief Check validity of a mesh.
   *
   * The returned error message string is heap-allocated and must be freed with
   * f3d_utils_string_free().
   *
   * @param mesh Mesh to validate.
   * @param error_message Pointer to receive error message if invalid.
   * @return 1 if valid, 0 if invalid.
   */
  F3D_EXPORT int f3d_mesh_is_valid(const f3d_mesh_t* mesh, char** error_message);

  /**
   * @brief Enumeration of light types.
   */
  typedef enum f3d_light_type_t
  {
    F3D_LIGHT_TYPE_HEADLIGHT = 1,
    F3D_LIGHT_TYPE_CAMERA_LIGHT = 2,
    F3D_LIGHT_TYPE_SCENE_LIGHT = 3
  } f3d_light_type_t;

  /**
   * @brief Structure describing the state of a light.
   */
  typedef struct f3d_light_state_t
  {
    f3d_light_type_t type;
    f3d_point3_t position;
    f3d_color_t color;
    f3d_vector3_t direction;
    int positional_light;
    double intensity;
    int switch_state;
  } f3d_light_state_t;

  /**
   * @brief Free a light state structure.
   *
   * @param light_state Light state to free.
   */
  F3D_EXPORT void f3d_light_state_free(f3d_light_state_t* light_state);

  /**
   * @brief Compare two light states for equality.
   *
   * @param a First light state.
   * @param b Second light state.
   * @return 1 if equal, 0 otherwise.
   */
  F3D_EXPORT int f3d_light_state_equal(const f3d_light_state_t* a, const f3d_light_state_t* b);

#ifdef __cplusplus
}
#endif

#endif // F3D_TYPES_C_API_H
