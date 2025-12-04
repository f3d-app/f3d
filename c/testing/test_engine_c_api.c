#include <engine_c_api.h>

#include <stdio.h>

int test_engine_c_api()
{
  int failed = 0;

  f3d_engine_autoload_plugins();
  f3d_engine_load_plugin("native");

  f3d_engine_t* engine = f3d_engine_create_none();
  if (engine == NULL)
  {
    puts("[ERROR] create_none() should return a valid engine");
    return 1;
  }

  const f3d_scene_t* scene = f3d_engine_get_scene(engine);
  if (scene == NULL)
  {
    puts("[ERROR] get_scene() should return a valid scene");
    failed++;
  }

  const f3d_options_t* options = f3d_engine_get_options(engine);
  if (options == NULL)
  {
    puts("[ERROR] get_options() should return valid options");
    failed++;
  }

  f3d_engine_set_cache_path(engine, "/tmp/f3d_test_cache");

  f3d_engine_delete(engine);

  return failed;
}
