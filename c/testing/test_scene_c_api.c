#include <engine_c_api.h>
#include <scene_c_api.h>
#include <types_c_api.h>
#include <utils_c_api.h>

#include <stdio.h>

int test_scene_c_api()
{
  f3d_engine_t* engine = f3d_engine_create_none();
  if (!engine)
  {
    puts("[ERROR] Failed to create engine");
    return 1;
  }

  f3d_scene_t* scene = f3d_engine_get_scene(engine);
  if (!scene)
  {
    puts("[ERROR] Failed to get scene");
    f3d_engine_delete(engine);
    return 1;
  }

  int supported = f3d_scene_supports(scene, "test.obj");
  (void)supported;

  f3d_scene_clear(scene);

  float points[] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f };
  unsigned int face_sides[] = { 3 };
  unsigned int face_indices[] = { 0, 1, 2 };

  f3d_mesh_t mesh = { 0 };
  mesh.points = points;
  mesh.points_count = 9;
  mesh.face_sides = face_sides;
  mesh.face_sides_count = 1;
  mesh.face_indices = face_indices;
  mesh.face_indices_count = 3;

  char* error_msg = NULL;
  int valid = f3d_mesh_is_valid(&mesh, &error_msg);
  (void)valid;
  if (error_msg)
  {
    f3d_utils_string_free(error_msg);
  }

  f3d_scene_add_mesh(scene, &mesh);

  f3d_scene_load_animation_time(scene, 0.5);
  double min_time, max_time;
  f3d_scene_animation_time_range(scene, &min_time, &max_time);
  unsigned int anim_count = f3d_scene_available_animations(scene);
  (void)anim_count;

  f3d_light_state_t light_state = { 0 };
  light_state.type = F3D_LIGHT_TYPE_HEADLIGHT;
  light_state.intensity = 1.0;
  light_state.switch_state = 1;

  int light_idx = f3d_scene_add_light(scene, &light_state);
  (void)light_idx;

  int light_count = f3d_scene_get_light_count(scene);
  (void)light_count;

  if (light_idx >= 0)
  {
    f3d_light_state_t* get_light = f3d_scene_get_light(scene, light_idx);
    if (get_light)
    {
      f3d_light_state_free(get_light);
    }

    f3d_light_state_t update_light = light_state;
    update_light.intensity = 2.0;
    f3d_scene_update_light(scene, light_idx, &update_light);

    f3d_scene_remove_light(scene, light_idx);
  }

  f3d_light_state_t light1 = { 0 };
  light1.type = F3D_LIGHT_TYPE_HEADLIGHT;
  light1.intensity = 1.0;

  f3d_light_state_t light2 = { 0 };
  light2.type = F3D_LIGHT_TYPE_CAMERA_LIGHT;
  light2.intensity = 1.0;

  int equal = f3d_light_state_equal(&light1, &light2);
  (void)equal;

  f3d_scene_remove_all_lights(scene);

  f3d_engine_delete(engine);
  return 0;
}
