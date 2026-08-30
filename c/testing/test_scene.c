#include <engine_c_api.h>
#include <scene_c_api.h>
#include <types_c_api.h>
#include <utils_c_api.h>

#include <stdio.h>
#include <string.h>

int test_scene()
{
  f3d_engine_autoload_plugins();

  f3d_engine_t* engine = f3d_engine_create(1);
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

  // Test adding files

  int supported = f3d_scene_supports(scene, "test.obj");
  (void)supported;

  int add_result = f3d_scene_add(scene, F3D_TESTING_DATA_DIR "cow.vtp");
  (void)add_result;

  const char* files[] = { F3D_TESTING_DATA_DIR "cow.vtp", F3D_TESTING_DATA_DIR "suzanne.obj" };
  int add_multiple_result = f3d_scene_add_multiple(scene, files, 2);
  (void)add_multiple_result;

  f3d_scene_clear(scene);

  // Test the added files tracking

  unsigned int added_count = 0;
  char** added_files = f3d_scene_get_added_files(scene, &added_count);
  if (added_count != 0)
  {
    puts("[ERROR] a cleared scene should have no added file");
    f3d_scene_free_added_files(added_files, added_count);
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_scene_free_added_files(added_files, added_count);

  f3d_scene_add(scene, F3D_TESTING_DATA_DIR "cow.vtp");
  added_files = f3d_scene_get_added_files(scene, &added_count);
  if (added_count != 1 || !added_files || !strstr(added_files[0], "cow.vtp"))
  {
    puts("[ERROR] scene should track the added file");
    f3d_scene_free_added_files(added_files, added_count);
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_scene_free_added_files(added_files, added_count);

  f3d_scene_clear(scene);

  // Test adding a mesh

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

  // no exception should be thrown here, it should return 0
  if (f3d_scene_add(scene, "invalid_path") == 1)
  {
    return 1;
  }

  const char* invalid_paths[] = { "invalid_path1", "invalid_path2", "invalid_path3" };

  if (f3d_scene_add_multiple(scene, invalid_paths, 3) == 1)
  {
    return 1;
  }

  // Test the rest of the API

  f3d_scene_load_animation_time(scene, 0.5);
  double min_time, max_time;
  f3d_scene_animation_time_range(scene, &min_time, &max_time);
  unsigned int anim_count = f3d_scene_available_animations(scene);
  (void)anim_count;
  unsigned int keyframes_number;
  double* keyframes = f3d_scene_get_animation_keyframes(scene, &keyframes_number);
  f3d_scene_free_animation_keyframes(keyframes);
  (void)keyframes;

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

  // Test the scene hierarchy

  f3d_scene_clear(scene);

  unsigned int null_count = 42;
  if (f3d_scene_get_scene_hierarchy(NULL, &null_count) || null_count != 0 ||
    f3d_scene_get_scene_hierarchy(scene, NULL) || f3d_scene_set_node_visibility(NULL, 0, 1))
  {
    puts("[ERROR] scene hierarchy API should handle NULL arguments");
    f3d_engine_delete(engine);
    return 1;
  }

  unsigned int node_count = 0;
  f3d_node_state_t* nodes = f3d_scene_get_scene_hierarchy(scene, &node_count);
  if (node_count != 0 || nodes)
  {
    puts("[ERROR] a cleared scene should have an empty scene hierarchy");
    f3d_scene_free_scene_hierarchy(nodes, node_count);
    f3d_engine_delete(engine);
    return 1;
  }

  if (f3d_scene_set_node_visibility(scene, 0, 0) != 0)
  {
    puts("[ERROR] setting node visibility should fail with an empty scene hierarchy");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_scene_add(scene, F3D_TESTING_DATA_DIR "mb/recursive/mb_0_0.vtu");
  nodes = f3d_scene_get_scene_hierarchy(scene, &node_count);
  if (node_count == 0 || !nodes)
  {
    puts("[ERROR] scene hierarchy should not be empty");
    f3d_scene_free_scene_hierarchy(nodes, node_count);
    f3d_engine_delete(engine);
    return 1;
  }

  if (nodes[0].id != 0 || nodes[0].parent_id != -1 || nodes[0].level != 0 || !nodes[0].visible ||
    !strstr(nodes[0].label, "mb_0_0.vtu"))
  {
    puts("[ERROR] unexpected scene hierarchy root node");
    f3d_scene_free_scene_hierarchy(nodes, node_count);
    f3d_engine_delete(engine);
    return 1;
  }

  if (f3d_scene_set_node_visibility(scene, 0, 0) != 1)
  {
    puts("[ERROR] failed to hide the scene hierarchy root node");
    f3d_scene_free_scene_hierarchy(nodes, node_count);
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_scene_free_scene_hierarchy(nodes, node_count);

  nodes = f3d_scene_get_scene_hierarchy(scene, &node_count);
  for (unsigned int i = 0; i < node_count; ++i)
  {
    if (nodes[i].visible)
    {
      puts("[ERROR] the whole subtree should be hidden");
      f3d_scene_free_scene_hierarchy(nodes, node_count);
      f3d_engine_delete(engine);
      return 1;
    }
  }

  if (f3d_scene_set_node_visibility(scene, (int)node_count, 1) != 0)
  {
    puts("[ERROR] setting node visibility should fail with an out of range index");
    f3d_scene_free_scene_hierarchy(nodes, node_count);
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_scene_free_scene_hierarchy(nodes, node_count);

  // Test the scene info

  f3d_scene_clear(scene);

  f3d_scene_info_t info;
  if (f3d_scene_get_scene_info(NULL, &info) || f3d_scene_get_scene_info(scene, NULL))
  {
    puts("[ERROR] scene info API should handle NULL arguments");
    f3d_engine_delete(engine);
    return 1;
  }

  if (!f3d_scene_get_scene_info(scene, &info) || info.number_of_files != 0 ||
    info.number_of_actors != 0 || info.number_of_points != 0 || info.number_of_cells != 0)
  {
    puts("[ERROR] a cleared scene should have zeroed scene info");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_scene_add(scene, F3D_TESTING_DATA_DIR "mb/recursive/mb_0_0.vtu");
  if (!f3d_scene_get_scene_info(scene, &info) || info.number_of_files != 1 ||
    info.number_of_actors <= 0 || info.number_of_points <= 0 || info.number_of_cells <= 0)
  {
    puts("[ERROR] unexpected scene info after adding a file");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_scene_info_t appended_info;
  f3d_scene_add(scene, F3D_TESTING_DATA_DIR "mb/recursive/mb_1_0.vtp");
  if (!f3d_scene_get_scene_info(scene, &appended_info) || appended_info.number_of_files != 2 ||
    appended_info.number_of_actors <= info.number_of_actors ||
    appended_info.number_of_points <= info.number_of_points ||
    appended_info.number_of_cells <= info.number_of_cells)
  {
    puts("[ERROR] scene info should increase when adding a file");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_scene_clear(scene);
  if (!f3d_scene_get_scene_info(scene, &info) || info.number_of_files != 0 ||
    info.number_of_actors != 0 || info.number_of_points != 0 || info.number_of_cells != 0)
  {
    puts("[ERROR] scene info should be cleared with the scene");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_engine_delete(engine);
  return 0;
}
