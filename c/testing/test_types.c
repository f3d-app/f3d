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

  f3d_color_t color;
  f3d_color_set(&color, 1.0, 0.5, 0.25);

  double r = f3d_color_r(&color);
  double g = f3d_color_g(&color);
  double b = f3d_color_b(&color);
  f3d_test_check_double(&test, "color r matches", r, 1.0, 1e-9);
  f3d_test_check_double(&test, "color g matches", g, 0.5, 1e-9);
  f3d_test_check_double(&test, "color b matches", b, 0.25, 1e-9);

  f3d_direction_t direction;
  f3d_direction_set(&direction, 1.0, 0.0, 0.0);

  double x = f3d_direction_x(&direction);
  double y = f3d_direction_y(&direction);
  double z = f3d_direction_z(&direction);
  f3d_test_check_double(&test, "direction x matches", x, 1.0, 1e-9);
  f3d_test_check_double(&test, "direction y matches", y, 0.0, 1e-9);
  f3d_test_check_double(&test, "direction z matches", z, 0.0, 1e-9);

  /* scale_x = scale_y = 1.0, so this reduces to a pure rotation + translation
   * matrix, matching the layout proven in TestSDKOptions.cxx for
   * f3d::transform2d_t(scale, translation, angle):
   * { cos(a), -sin(a), tx, sin(a), cos(a), ty, 0, 0, 1 } */
  f3d_transform2d_t transform;
  f3d_transform2d_create(&transform, 1.0, 1.0, 10.0, 20.0, 45.0);

  const double angle_rad = 45.0 * M_PI / 180.0;
  const double expected[9] = { cos(angle_rad), -sin(angle_rad), 10.0, sin(angle_rad),
    cos(angle_rad), 20.0, 0.0, 0.0, 1.0 };

  for (int i = 0; i < 9; ++i)
  {
    char label[64];
    snprintf(label, sizeof(label), "transform matrix[%d] matches", i);
    f3d_test_check_double(&test, label, transform.data[i], expected[i], 1e-9);
  }

  f3d_colormap_t colormap;
  colormap.data = NULL;
  colormap.count = 0;
  f3d_colormap_free(&colormap);
  f3d_test_check(&test, "colormap_free handles null data without crashing", 1);

  return f3d_test_result(&test);
}
