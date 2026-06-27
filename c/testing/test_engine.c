#include <engine_c_api.h>
#include <interactor_c_api.h>
#include <scene_c_api.h>
#include <window_c_api.h>

#include <stdio.h>
#include <string.h>

int test_engine()
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

  // f3d load invalid plugin should not crash
  f3d_engine_load_plugin("inexistent_plugin");

  char** plugins_list = f3d_engine_get_plugins_list("inexistent");
  (void)plugins_list;

  // invalid set reader option should not crash
  f3d_engine_set_reader_option("invalid.option", "value");

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

  // Statefile round trip: set an option, add a file, save the state then restore it into a fresh
  // engine and check both the option and the loaded scene are restored
  f3d_options_set_as_bool(f3d_engine_get_options(engine), "ui.scalar_bar", 1);
  if (f3d_scene_add(f3d_engine_get_scene(engine), F3D_TESTING_DATA_DIR "cow.vtp") != 1)
  {
    puts("[ERROR] failed to add a file before saving the statefile");
    f3d_engine_delete(engine);
    return 1;
  }

  // String based round trip
  const char* state = f3d_engine_save_statefile_to_string(engine);
  if (!state)
  {
    puts("[ERROR] save_statefile_to_string() should return a valid string");
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_engine_t* engine_str = f3d_engine_create(1);
  if (!f3d_engine_load_statefile_from_string(engine_str, state))
  {
    puts("[ERROR] load_statefile_from_string() should succeed");
    f3d_engine_free_string(state);
    f3d_engine_delete(engine_str);
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_engine_free_string(state);
  if (!f3d_options_get_as_bool(f3d_engine_get_options(engine_str), "ui.scalar_bar"))
  {
    puts("[ERROR] options should be restored from the statefile string");
    f3d_engine_delete(engine_str);
    f3d_engine_delete(engine);
    return 1;
  }
  unsigned int str_count = 0;
  char** str_files = f3d_scene_get_added_files(f3d_engine_get_scene(engine_str), &str_count);
  if (str_count != 1 || !str_files || !strstr(str_files[0], "cow.vtp"))
  {
    puts("[ERROR] scene should be restored from the statefile string");
    f3d_scene_free_added_files(str_files, str_count);
    f3d_engine_delete(engine_str);
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_scene_free_added_files(str_files, str_count);
  f3d_engine_delete(engine_str);

  // File based round trip
  const char* statefile_path = "test_engine_c_api_statefile.json";
  if (!f3d_engine_save_statefile(engine, statefile_path))
  {
    puts("[ERROR] save_statefile() should succeed");
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_engine_t* engine_file = f3d_engine_create(1);
  if (!f3d_engine_load_statefile(engine_file, statefile_path))
  {
    puts("[ERROR] load_statefile() should succeed");
    f3d_engine_delete(engine_file);
    f3d_engine_delete(engine);
    return 1;
  }
  unsigned int file_count = 0;
  char** file_files = f3d_scene_get_added_files(f3d_engine_get_scene(engine_file), &file_count);
  if (file_count != 1 || !file_files || !strstr(file_files[0], "cow.vtp"))
  {
    puts("[ERROR] scene should be restored from the statefile file");
    f3d_scene_free_added_files(file_files, file_count);
    f3d_engine_delete(engine_file);
    f3d_engine_delete(engine);
    return 1;
  }
  f3d_scene_free_added_files(file_files, file_count);
  f3d_engine_delete(engine_file);

  // Loading an invalid statefile should fail gracefully
  if (f3d_engine_load_statefile(engine, "/does/not/exist/state.json"))
  {
    puts("[ERROR] load_statefile() should fail on a missing file");
    f3d_engine_delete(engine);
    return 1;
  }
  if (f3d_engine_load_statefile_from_string(engine, "{ not valid json"))
  {
    puts("[ERROR] load_statefile_from_string() should fail on invalid content");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_engine_delete(engine);
  return 0;
}
