#include <types_c_api.h>

#include <stdio.h>

int test_types()
{
  f3d_color_t color;
  f3d_color_set(&color, 1.0, 0.5, 0.25);

  double r = f3d_color_r(&color);
  (void)r;
  double g = f3d_color_g(&color);
  (void)g;
  double b = f3d_color_b(&color);
  (void)b;

  f3d_direction_t direction;
  f3d_direction_set(&direction, 1.0, 0.0, 0.0);

  double x = f3d_direction_x(&direction);
  (void)x;
  double y = f3d_direction_y(&direction);
  (void)y;
  double z = f3d_direction_z(&direction);
  (void)z;

  f3d_transform2d_t transform;
  f3d_transform2d_create(&transform, 1.0, 1.0, 10.0, 20.0, 45.0);

  f3d_colormap_t colormap;
  colormap.data = NULL;
  colormap.count = 0;
  f3d_colormap_free(&colormap);

  return 0;
}
