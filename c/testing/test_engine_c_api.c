#include <engine_c_api.h>
#include <interactor_c_api.h>
#include <scene_c_api.h>
#include <window_c_api.h>

#include <stdio.h>

int test_engine_c_api()
{
  f3d_engine_t* engine = f3d_engine_create(1);
  if (!engine)
  {
    puts("[ERROR] Failed to create engine");
    return 1;
  }

  const f3d_scene_t* scene = f3d_engine_get_scene(engine);
  if (!scene)
  {
    puts("[ERROR] get_scene() should return a valid scene");
    f3d_engine_delete(engine);
    return 1;
  }

  const f3d_options_t* options = f3d_engine_get_options(engine);
  if (!options)
  {
    puts("[ERROR] get_options() should return valid options");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_window_t* window = f3d_engine_get_window(engine);
  (void)window;

  f3d_interactor_t* interactor = f3d_engine_get_interactor(engine);
  (void)interactor;

  f3d_engine_set_cache_path(engine, "/tmp/f3d_test_cache");

  f3d_engine_autoload_plugins();
  f3d_engine_load_plugin("native");

  f3d_engine_set_reader_option("QuakeMDL.skin_index", "0");

  f3d_engine_t* engine_none = f3d_engine_create_none();
  if (engine_none)
  {
    f3d_engine_delete(engine_none);
  }

  f3d_engine_delete(engine);
  return 0;
}
