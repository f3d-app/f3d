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

  const f3d_color_t near_color = { { 1.0, 0.5, 0.2500005 } };
  const f3d_color_t different_color = { { 1.0, 0.5, 0.250002 } };
  const f3d_color_t zero_color = { { 0.0, 0.0, 0.0 } };
  const f3d_color_t boundary_color = { { 0.0, 0.0, 1e-6 } };

  if (f3d_color_compare(&color, &color, 0.0) != 1 ||
    f3d_color_compare(&color, &near_color, 1e-6) != 1 ||
    f3d_color_compare(&near_color, &color, 1e-6) != 1 ||
    f3d_color_compare(&color, &different_color, 1e-6) != 0 ||
    f3d_color_compare(&zero_color, &boundary_color, 1e-6) != 1 ||
    f3d_color_compare(NULL, &color, 1e-6) != 0 || f3d_color_compare(&color, NULL, 1e-6) != 0)
  {
    fprintf(stderr, "Color comparison failed\n");
    return 1;
  }

  f3d_direction_t direction;
  f3d_direction_set(&direction, 1.0, 0.0, 0.0);

  double x = f3d_direction_x(&direction);
  (void)x;
  double y = f3d_direction_y(&direction);
  (void)y;
  double z = f3d_direction_z(&direction);
  (void)z;

  const f3d_direction_t near = { { 1.0000005, 0.0, 0.0 } };
  const f3d_direction_t different = { { 1.0, 0.01, 0.0 } };

  if (f3d_direction_compare(&direction, &direction, 0.0) != 1 ||
    f3d_direction_compare(&direction, &near, 1e-6) != 1 ||
    f3d_direction_compare(&direction, &different, 1e-6) != 0 ||
    f3d_direction_compare(NULL, &direction, 1e-6) != 0 ||
    f3d_direction_compare(&direction, NULL, 1e-6) != 0)
  {
    fprintf(stderr, "Direction comparison failed\n");
    return 1;
  }

  f3d_transform2d_t transform;
  f3d_transform2d_create(&transform, 1.0, 1.0, 10.0, 20.0, 45.0);

  f3d_transform2d_t near_transform = transform;
  near_transform.data[8] += 0.0000005;
  f3d_transform2d_t different_transform = transform;
  different_transform.data[8] += 0.000002;
  const f3d_transform2d_t zero_transform = { { 0.0 } };
  f3d_transform2d_t boundary_transform = zero_transform;
  boundary_transform.data[8] = 1e-6;

  if (f3d_transform2d_compare(&transform, &transform, 0.0) != 1 ||
    f3d_transform2d_compare(&transform, &near_transform, 1e-6) != 1 ||
    f3d_transform2d_compare(&near_transform, &transform, 1e-6) != 1 ||
    f3d_transform2d_compare(&transform, &different_transform, 1e-6) != 0 ||
    f3d_transform2d_compare(&zero_transform, &boundary_transform, 1e-6) != 1 ||
    f3d_transform2d_compare(NULL, &transform, 1e-6) != 0 ||
    f3d_transform2d_compare(&transform, NULL, 1e-6) != 0)
  {
    fprintf(stderr, "Transform comparison failed\n");
    return 1;
  }

  f3d_colormap_t colormap;
  colormap.data = NULL;
  colormap.count = 0;
  f3d_colormap_destroy(&colormap);

  return 0;
}
