#include "pseudo_unit_test.h"
#include <types_c_api.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int test_types()
{
  f3d_test_t test;
  f3d_test_init(&test);

  // f3d_color_t testers
  f3d_color_t color = { {1.0, 0.5, 0.25} };

  f3d_test_check_double(&test, "color r getter works", f3d_color_r(&color), 1.0);
  f3d_test_check_double(&test, "color g getter works", f3d_color_g(&color), 0.5);
  f3d_test_check_double(&test, "color b getter works", f3d_color_b(&color), 0.25);

  f3d_test_check_double(&test, "color r handles NULL", f3d_color_r(NULL), 0.0);
  f3d_test_check_double(&test, "color g handles NULL", f3d_color_g(NULL), 0.0);
  f3d_test_check_double(&test, "color b handles NULL", f3d_color_b(NULL), 0.0);

  f3d_color_set(&color, 0.5, 1.0, 1.5);
  const double expected_color[3] = { 0.5, 1.0, 1.5 };
  f3d_test_check_vec3(&test, "color setter works", color.data, expected_color);
  f3d_color_set(NULL, 1.0, 2.0, 3.0);

  // f3d_direction_t testers
  f3d_direction_t direction = { {0.2, 0.4, 0.6} };

  f3d_test_check_double(&test, "direction x getter works", f3d_direction_x(&direction), 0.2);
  f3d_test_check_double(&test, "direction y getter works", f3d_direction_y(&direction), 0.4);
  f3d_test_check_double(&test, "direction z getter works", f3d_direction_z(&direction), 0.6);

  f3d_test_check_double(&test, "direction x handles NULL", f3d_direction_x(NULL), 0.0);
  f3d_test_check_double(&test, "direction y handles NULL", f3d_direction_y(NULL), 0.0);
  f3d_test_check_double(&test, "direction z handles NULL", f3d_direction_z(NULL), 0.0);

  f3d_direction_set(&direction, 1.2, 1.4, 1.6);
  const double expected_direction[3] = { 1.2, 1.4, 1.6 };
  f3d_test_check_vec3(&test, "direction setter works", direction.data, expected_direction);
  f3d_direction_set(NULL, 1.0, 2.0, 3.0);

  // f3d_transform2d_t testers
  f3d_transform2d_t transform;
  f3d_transform2d_create(
    &transform,
    1.0,   // scale_x
    1.0,   // scale_y
    10.0,  // translate_x
    20.0,  // translate_y
    45.0); // angle

  const double angle_rad = 45.0 * M_PI / 180.0;
  const double expected_transform[9] = {
    cos(angle_rad), -sin(angle_rad), 10.0,
    sin(angle_rad),  cos(angle_rad), 20.0,
    0.0,             0.0,            1.0
  };

  for (int i = 0; i < 9; ++i)
  {
    char label[64];
    snprintf(label, sizeof(label), "transform matrix[%d] matches", i);

    f3d_test_check_double(&test, label, transform.data[i], expected_transform[i]);
  }
  // NULL handling 
  f3d_transform2d_create(NULL, 1.0, 1.0, 10.0, 20.0, 45.0);

  // f3d_colormap_free testers
  f3d_colormap_t colormap;
  colormap.count = 3;
  colormap.data = malloc(colormap.count * sizeof(double));

  colormap.data[0] = 0.1;
  colormap.data[1] = 0.2;
  colormap.data[2] = 0.3;

  f3d_colormap_free(&colormap);

  f3d_test_check(&test, "colormap data reset", colormap.data == NULL);
  f3d_test_check_int(&test, "colormap count reset", colormap.count, 0);

  //f3d_mesh_t testers are yet to be implemented

  //f3d_light_state_t testers are yet to be implemented
  return f3d_test_result(&test);
}
