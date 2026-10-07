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
  int add_result = f3d_scene_add(scene, F3D_TESTING_DATA_DIR "soldier_animations.mdl");
  (void)add_result;

  // Test the animation API
  f3d_animation_t* anim = f3d_scene_get_animation(scene);

  f3d_animation_load_time(anim, 0.5);

  double min_time, max_time;
  f3d_animation_get_time_range(anim, &min_time, &max_time);
  if (min_time != 0 || max_time - 0.8 > 1e-10)
  {
    puts("[ERROR] Failed to recover expected time range");
    f3d_engine_destroy(engine);
    return 1;
  }

  unsigned int anim_count = f3d_animation_count(anim);
  if (anim_count != 10)
  {
    puts("[ERROR] Failed to recover expected animation count");
    f3d_engine_destroy(engine);
    return 1;
  }

  unsigned int key_frames_number;
  double* key_frames = f3d_animation_get_key_frames(anim, &key_frames_number);
  if (key_frames_number != 9 || key_frames[1] != 0.1)
  {
    puts("[ERROR] Failed to recover expected animation key_frames");
    f3d_engine_destroy(engine);
    return 1;
  }
  f3d_animation_destroy_key_frames(key_frames);

  char* name = f3d_animation_get_current_name(anim);
  if (strcmp(name, "stand") != 0)
  {
    puts("[ERROR] Failed to recover expected current animation name");
    f3d_engine_destroy(engine);
    return 1;
  }
  f3d_animation_destroy_string(name);

  name = f3d_animation_get_name(anim, 1);
  if (strcmp(name, "dead") != 0)
  {
    puts("[ERROR] Failed to recover expected animation name");
    f3d_engine_destroy(engine);
    return 1;
  }
  f3d_animation_destroy_string(name);

  int count;
  char** names = f3d_animation_get_names(anim, &count);
  if (count != 10 || strcmp(names[1], "dead") != 0)
  {
    puts("[ERROR] Failed to recover expected animation names");
    f3d_engine_destroy(engine);
    return 1;
  }
  f3d_animation_destroy_string_array(names, count);

  f3d_engine_destroy(engine);
  return 0;
}
