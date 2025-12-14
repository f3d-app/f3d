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

  char** plugins_list = f3d_engine_get_plugins_list("inexistent");
  (void)plugins_list;

  f3d_engine_set_reader_option("QuakeMDL.skin_index", "0");

  char** option_names = f3d_engine_get_all_reader_option_names();
  f3d_engine_free_string_array(option_names);

  f3d_backend_info_t* backends = f3d_engine_get_rendering_backend_list(NULL);
  f3d_engine_free_backend_list(backends);

  f3d_lib_info_t* lib_info = f3d_engine_get_lib_info();
  if (!lib_info)
  {
    puts("[ERROR] get_lib_info() should return valid info");
    f3d_engine_delete(engine);
    return 1;
  }
  if (!lib_info->version || !lib_info->vtk_version)
  {
    puts("[ERROR] lib_info should have version and vtk_version");
    f3d_engine_free_lib_info(lib_info);
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_engine_free_lib_info(lib_info);

  int reader_count = 0;
  f3d_reader_info_t* readers = f3d_engine_get_readers_info(&reader_count);
  if (!readers || reader_count == 0)
  {
    puts("[ERROR] get_readers_info() should return at least one reader");
    f3d_engine_free_readers_info(readers);
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_engine_free_readers_info(readers);

  f3d_options_t* new_options = f3d_engine_get_options(engine);
  f3d_engine_set_options(engine, new_options);

  f3d_engine_t* engine_none = f3d_engine_create_none();
  if (engine_none)
  {
    f3d_engine_delete(engine_none);
  }

  f3d_engine_delete(engine);
  return 0;
}
