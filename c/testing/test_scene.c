#include "pseudo_unit_test.h"

#include <engine_c_api.h>
#include <scene_c_api.h>
#include <types_c_api.h>
#include <utils_c_api.h>

#include <stdio.h>
#include <string.h>

int test_scene()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_engine_autoload_plugins();

  f3d_engine_t* engine = f3d_engine_create(1);
  f3d_test_check(&test, "engine created", engine != NULL);
  if (!engine)
  {
    return f3d_test_result(&test);
  }

  f3d_scene_t* scene = f3d_engine_get_scene(engine);
  f3d_test_check(&test, "scene retrieved", scene != NULL);
  if (!scene)
  {
    f3d_engine_delete(engine);
    return f3d_test_result(&test);
  }

  int supported = f3d_scene_supports(scene, F3D_TESTING_DATA_DIR "suzanne.obj");
  f3d_test_check_int(&test, "obj format is supported", supported, 1);

  int not_supported_missing = f3d_scene_supports(scene, "does_not_exist.obj");
  f3d_test_check_int(&test, "obj format with missing file is not supported", not_supported_missing, 0);

  int add_result = f3d_scene_add(scene, F3D_TESTING_DATA_DIR "cow.vtp");
  f3d_test_check_int(&test, "add valid file returns 1 (success)", add_result, 1);

  const char* files[] = { F3D_TESTING_DATA_DIR "cow.vtp", F3D_TESTING_DATA_DIR "suzanne.obj" };
  int add_multiple_result = f3d_scene_add_multiple(scene, files, 2);
  f3d_test_check_int(
    &test, "add_multiple valid files returns 1 (success)", add_multiple_result, 1);

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
  f3d_test_check_int(&test, "well-formed triangle mesh is valid", valid, 1);
  f3d_test_check(&test, "no error message for valid mesh", error_msg == NULL);
  if (error_msg)
  {
    f3d_utils_string_free(error_msg);
  }

  int add_mesh_result = f3d_scene_add_mesh(scene, &mesh);
  f3d_test_check_int(&test, "add_mesh for valid mesh returns 1 (success)", add_mesh_result, 1);

  f3d_test_check_int(
    &test, "add invalid path returns 0 (failure)", f3d_scene_add(scene, "invalid_path"), 0);

  const char* invalid_paths[] = { "invalid_path1", "invalid_path2", "invalid_path3" };
  f3d_test_check_int(&test, "add_multiple invalid paths returns 0 (failure)",
    f3d_scene_add_multiple(scene, invalid_paths, 3), 0);

  f3d_scene_load_animation_time(scene, 0.5);
  double min_time, max_time;
  f3d_scene_animation_time_range(scene, &min_time, &max_time);
  unsigned int anim_count = f3d_scene_available_animations(scene);
  (void)anim_count;
  unsigned int keyframes_number;
  double* keyframes = f3d_scene_get_animation_keyframes(scene, &keyframes_number);
  f3d_scene_free_animation_keyframes(keyframes);
  (void)keyframes;

  f3d_scene_remove_all_lights(scene);
  int light_count_empty = f3d_scene_get_light_count(scene);
  f3d_test_check_int(&test, "light count is 0 after remove_all_lights", light_count_empty, 0);

  f3d_light_state_t light_state = { 0 };
  light_state.type = F3D_LIGHT_TYPE_HEADLIGHT;
  light_state.intensity = 1.0;
  light_state.switch_state = 1;

  int light_idx = f3d_scene_add_light(scene, &light_state);
  f3d_test_check_int(&test, "add_light on empty scene returns index 0", light_idx, 0);

  int light_count_after_add = f3d_scene_get_light_count(scene);
  f3d_test_check_int(&test, "light count is 1 after add", light_count_after_add, 1);

  f3d_light_state_t* get_light = f3d_scene_get_light(scene, light_idx);
  f3d_test_check(&test, "get_light returns non-null after add", get_light != NULL);
  if (get_light)
  {
    f3d_test_check_int(
      &test, "retrieved light type matches", (int)get_light->type, (int)F3D_LIGHT_TYPE_HEADLIGHT);
    f3d_test_check_double(
      &test, "retrieved light intensity matches", get_light->intensity, 1.0);
    f3d_test_check_int(
      &test, "retrieved light switch_state matches", get_light->switch_state, 1);
    f3d_light_state_free(get_light);
  }

  f3d_light_state_t update_light = light_state;
  update_light.intensity = 2.0;
  int update_result = f3d_scene_update_light(scene, light_idx, &update_light);
  f3d_test_check_int(&test, "update_light on valid index returns 1 (success)", update_result, 1);

  f3d_light_state_t* get_updated_light = f3d_scene_get_light(scene, light_idx);
  f3d_test_check(&test, "get_light returns non-null after update", get_updated_light != NULL);
  if (get_updated_light)
  {
    f3d_test_check_double(
      &test, "updated light intensity matches", get_updated_light->intensity, 2.0);
    f3d_light_state_free(get_updated_light);
  }

  f3d_light_state_t* invalid_light = f3d_scene_get_light(scene, 99);
  if (invalid_light)
  {
    f3d_light_state_free(invalid_light);
  }

  int remove_result = f3d_scene_remove_light(scene, light_idx);
  f3d_test_check_int(&test, "remove_light on valid index returns 1 (success)", remove_result, 1);

  int light_count_after_remove = f3d_scene_get_light_count(scene);
  f3d_test_check_int(&test, "light count is 0 after remove", light_count_after_remove, 0);

  f3d_light_state_t light1 = { 0 };
  light1.type = F3D_LIGHT_TYPE_HEADLIGHT;
  light1.intensity = 1.0;

  f3d_light_state_t light2 = { 0 };
  light2.type = F3D_LIGHT_TYPE_CAMERA_LIGHT;
  light2.intensity = 1.0;

  int equal = f3d_light_state_equal(&light1, &light2);
  f3d_test_check_int(&test, "lights with different types are not equal", equal, 0);

  f3d_light_state_t light3 = { 0 };
  light3.type = F3D_LIGHT_TYPE_HEADLIGHT;
  light3.intensity = 1.0;
  int equal_identical = f3d_light_state_equal(&light1, &light3);
  f3d_test_check_int(&test, "identical lights are equal", equal_identical, 1);

  f3d_scene_remove_all_lights(scene);
  int light_count_after_remove_all = f3d_scene_get_light_count(scene);
  f3d_test_check_int(
    &test, "light count is 0 after remove_all_lights", light_count_after_remove_all, 0);

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}
