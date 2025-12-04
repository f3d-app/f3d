#include <scene_c_api.h>
#include <types_c_api.h>
#include <utils_c_api.h>

#include <stdio.h>
#include <stdlib.h>

int test_scene_c_api()
{
  int failed = 0;

  float points[] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f };
  unsigned int face_sides[] = { 3 };
  unsigned int face_indices[] = { 0, 1, 2 };

  f3d_mesh_t valid_mesh = { 0 };
  valid_mesh.points = points;
  valid_mesh.points_count = 9;
  valid_mesh.normals = NULL;
  valid_mesh.normals_count = 0;
  valid_mesh.texture_coordinates = NULL;
  valid_mesh.texture_coordinates_count = 0;
  valid_mesh.face_sides = face_sides;
  valid_mesh.face_sides_count = 1;
  valid_mesh.face_indices = face_indices;
  valid_mesh.face_indices_count = 3;

  char* error_msg = NULL;
  if (!f3d_mesh_is_valid(&valid_mesh, &error_msg))
  {
    printf("[ERROR] Valid mesh reported as invalid: %s\n", error_msg ? error_msg : "unknown");
    if (error_msg)
    {
      f3d_utils_string_free(error_msg);
    }
    failed++;
  }

  float invalid_points[] = { 0.0f, 0.0f };
  f3d_mesh_t invalid_mesh = { 0 };
  invalid_mesh.points = invalid_points;
  invalid_mesh.points_count = 2;

  error_msg = NULL;
  if (f3d_mesh_is_valid(&invalid_mesh, &error_msg))
  {
    puts("[ERROR] Invalid mesh (points_count not multiple of 3) reported as valid");
    failed++;
  }
  if (error_msg)
  {
    f3d_utils_string_free(error_msg);
  }

  f3d_light_state_t light1 = { 0 };
  light1.type = F3D_LIGHT_TYPE_HEADLIGHT;
  light1.intensity = 1.0;

  f3d_light_state_t light2 = { 0 };
  light2.type = F3D_LIGHT_TYPE_CAMERA_LIGHT;
  light2.intensity = 1.0;

  if (f3d_light_state_equal(&light1, &light2))
  {
    puts("[ERROR] Light states with different types reported as equal");
    failed++;
  }

  return failed;
}
