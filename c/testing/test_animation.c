#include <animation_c_api.h>
#include <engine_c_api.h>
#include <scene_c_api.h>
#include <types_c_api.h>
#include <utils_c_api.h>

#include <stdio.h>
#include <string.h>

int test_animation()
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
    f3d_engine_destroy(engine);
    return 1;
  }

  // Add an animated file
  int add_result = f3d_scene_add(scene, F3D_TESTING_DATA_DIR "BoxAnimated.gltf");
  (void)add_result;

  // Test the animation API
  f3d_animation_t* anim = f3d_scene_get_animation(scene);

  f3d_animation_load_time(anim, 0.5);

  double min_time, max_time;
  f3d_animation_time_range(anim, &min_time, &max_time);
  if (min_time != 0 || max_time - 3.708330 > 1e-10)
  {
    puts("[ERROR] Failed to recover expected time range");
    return 1;
  }

  unsigned int anim_count = f3d_animation_count(anim);
  if (anim_count != 1)
  {
    puts("[ERROR] Failed to recover expected animation count");
    return 1;
  }

  unsigned int keyframes_number;
  double* keyframes = f3d_animation_keyframes(anim, &keyframes_number);
  if (keyframes_number != 4 || keyframes[0] != 0)
  {
    puts("[ERROR] Failed to recover expected animation keyframes");
    return 1;
  }
  f3d_animation_destroy_keyframes(keyframes);

  char* name = f3d_animation_get_name(anim, 0);
  if (strcmp(name, "unnamed_0") != 0)
  {
    puts("[ERROR] Failed to recover expected animation name");
    return 1;
  }
  f3d_animation_destroy_string(name);

  int count;
  char** names = f3d_animation_get_names(anim, &count);
  if (count != 1 || strcmp(names[0], "unnamed_0") != 0)
  {
    puts("[ERROR] Failed to recover expected animation names");
    return 1;
  }
  f3d_animation_destroy_string_array(names, count);

  f3d_engine_destroy(engine);
  return 0;
}
