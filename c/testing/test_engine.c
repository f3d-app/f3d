#include "pseudo_unit_test.h"
#include <engine_c_api.h>
#include <interactor_c_api.h>
#include <scene_c_api.h>
#include <window_c_api.h>
#include <string.h>

static void* test_get_proc_address(const char* name)
{
  (void)name;
  return NULL;
}

int test_engine()
{
  f3d_test_t test;
  f3d_test_init(&test);

  // f3d_engine_create tests
  f3d_engine_t* engine = f3d_engine_create(1);
  f3d_test_check_ptr(&test, "engine_create(offscreen)", engine);

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  engine = f3d_engine_create(0);
  f3d_test_check_ptr(&test, "engine_create(onscreen)", engine);

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_none tests
  engine = f3d_engine_create_none();
  f3d_test_check_ptr(&test, "engine_create_none()", engine);

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_glx tests
  engine = f3d_engine_create_glx(1);

  #ifdef F3D_TESTING_ENABLE_GLX_TESTS
  f3d_test_check_ptr(&test, "engine_create_glx(offscreen)", engine);
  #else
  f3d_test_check_null(&test, "engine_create_glx() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  engine = f3d_engine_create_glx(0);

  #ifdef F3D_TESTING_ENABLE_GLX_TESTS
  f3d_test_check_ptr(&test, "engine_create_glx(onscreen)", engine);
  #else
  f3d_test_check_null(&test, "engine_create_glx() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_wgl tests
  engine = f3d_engine_create_wgl(1);

  #ifdef F3D_TESTING_ENABLE_WGL_TESTS
  f3d_test_check_ptr(&test, "engine_create_wgl(offscreen)", engine);
  #else
  f3d_test_check_null(&test, "engine_create_wgl() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  engine = f3d_engine_create_wgl(0);

  #ifdef F3D_TESTING_ENABLE_WGL_TESTS
  f3d_test_check_ptr(&test, "engine_create_wgl(onscreen)", engine);
  #else
  f3d_test_check_null(&test, "engine_create_wgl() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_egl tests
  engine = f3d_engine_create_egl();

  #ifdef F3D_TESTING_ENABLE_EGL_TESTS
  f3d_test_check_ptr(&test, "engine_create_egl()", engine);
  #else
  f3d_test_check_null(&test, "engine_create_egl() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_osmesa tests
  engine = f3d_engine_create_osmesa();

  #ifdef F3D_TESTING_ENABLE_OSMESA_TESTS
  f3d_test_check_ptr(&test, "engine_create_osmesa()", engine);
  #else
  f3d_test_check_null(&test, "engine_create_osmesa() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_external tests
  f3d_test_check_null(&test, "engine_create_external(NULL)", f3d_engine_create_external(NULL));

  engine = f3d_engine_create_external(test_get_proc_address);
  f3d_test_check_null(&test, "engine_create_external(invalid loader)", engine);

  // f3d_engine_create_external_glx tests
  engine = f3d_engine_create_external_glx();

  #ifdef F3D_TESTING_ENABLE_GLX_TESTS
  f3d_test_check_ptr(&test, "engine_create_external_glx()", engine);
  #else
  f3d_test_check_null(&test, "engine_create_external_glx() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_external_wgl tests
  engine = f3d_engine_create_external_wgl();

  #ifdef F3D_TESTING_ENABLE_WGL_TESTS
  f3d_test_check_ptr(&test, "engine_create_external_wgl()", engine);
  #else
  f3d_test_check_null(&test, "engine_create_external_wgl() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_external_cocoa tests
  engine = f3d_engine_create_external_cocoa();

  #ifdef __APPLE__
  f3d_test_check_ptr(&test, "engine_create_external_cocoa()", engine);
  #else
  f3d_test_check_null(&test, "engine_create_external_cocoa() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_external_egl tests
  engine = f3d_engine_create_external_egl();

  #ifdef F3D_TESTING_ENABLE_EGL_TESTS
  f3d_test_check_ptr(&test, "engine_create_external_egl()", engine);
  #else
  f3d_test_check_null(&test, "engine_create_external_egl() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_create_external_osmesa tests
  engine = f3d_engine_create_external_osmesa();

  #ifdef F3D_TESTING_ENABLE_OSMESA_TESTS
  f3d_test_check_ptr(&test, "engine_create_external_osmesa()", engine);
  #else
  f3d_test_check_null(&test, "engine_create_external_osmesa() unavailable", engine);
  #endif

  if (engine)
  {
    f3d_engine_delete(engine);
  }

  // f3d_engine_delete tests
  f3d_engine_delete(NULL);
  f3d_test_check(&test, "engine_delete(NULL)", 1);

  engine = f3d_engine_create(1);
  f3d_test_check_ptr(&test, "engine_create for delete", engine);

  f3d_engine_delete(engine);
  f3d_test_check(&test, "engine_delete(valid)", 1);

  // f3d_engine_set_cache_path tests
  f3d_test_check_int(&test, "engine_set_cache_path(NULL engine)",
    f3d_engine_set_cache_path(NULL, F3D_TESTING_TEMP_DIR), 0);

  f3d_test_check_int(&test, "engine_set_cache_path(NULL path)",
    f3d_engine_set_cache_path(engine, NULL), 0);

  f3d_test_check_int(&test, "engine_set_cache_path(valid)",
    f3d_engine_set_cache_path(engine, F3D_TESTING_TEMP_DIR), 1);

  f3d_test_check_int(&test, "engine_set_cache_path(empty)",
    f3d_engine_set_cache_path(engine, ""), 0);

  char long_path[259];
  long_path[0] = '/';
  memset(long_path + 1, 'x', 257);
  long_path[258] = '\0';

  f3d_test_check_int(&test, "engine_set_cache_path(long path)",
    f3d_engine_set_cache_path(engine, long_path), 0);

  // f3d_engine_set_options and f3d_engine_get_options tests
  f3d_options_t* options = f3d_options_create();
  f3d_test_check_ptr(&test, "options created", options);

  f3d_engine_set_options(NULL, options);
  f3d_test_check(&test, "engine_set_options(NULL engine)", 1);

  f3d_engine_set_options(engine, NULL);
  f3d_test_check(&test, "engine_set_options(NULL options)", 1);

  f3d_test_check_null(&test, "engine_get_options(NULL)", f3d_engine_get_options(NULL));

  f3d_options_set_as_bool(options, "model.scivis.cells", 1);

  f3d_engine_set_options(engine, options);

  f3d_options_t* engine_options = f3d_engine_get_options(engine);
  f3d_test_check_ptr(&test, "engine_get_options(valid)", engine_options);

  f3d_test_check_int(&test, "engine options updated",
    f3d_options_get_as_bool(engine_options, "model.scivis.cells"), 1);

  f3d_options_delete(options);

  // f3d_engine_get_window tests
  f3d_test_check_null(&test, "engine_get_window(NULL)", f3d_engine_get_window(NULL));

  f3d_test_check_ptr(&test, "engine_get_window(valid)", f3d_engine_get_window(engine));

  f3d_engine_t* none_engine = f3d_engine_create_none();
  f3d_test_check_ptr(&test, "engine_create_none()", none_engine);

  f3d_test_check_null(&test, "engine_get_window(no window)", f3d_engine_get_window(none_engine));

  f3d_engine_delete(none_engine);

  // f3d_engine_get_scene tests
  f3d_test_check_null(&test, "engine_get_scene(NULL)", f3d_engine_get_scene(NULL));
  f3d_test_check_ptr(&test, "engine_get_scene(valid)", f3d_engine_get_scene(engine));

  // f3d_engine_get_interactor tests
  f3d_test_check_null(&test, "engine_get_interactor(NULL)", f3d_engine_get_interactor(NULL));
  f3d_test_check_ptr(&test, "engine_get_interactor(valid)", f3d_engine_get_interactor(engine));

  none_engine = f3d_engine_create_none();
  f3d_test_check_ptr(&test, "engine_create_none()", none_engine);

  f3d_test_check_null(&test, "engine_get_interactor(no interactor)", f3d_engine_get_interactor(none_engine));
  f3d_engine_delete(none_engine);

  // f3d_engine_load_plugin tests
  f3d_test_check_int(&test, "engine_load_plugin(NULL)", f3d_engine_load_plugin(NULL), 0);
  f3d_test_check_int(&test, "engine_load_plugin(invalid library)", f3d_engine_load_plugin("invalid.so"), 0);

  char long_name[264];
  long_name[0] = '/';
  memset(long_name + 1, 'x', 257);
  strcpy(long_name + 258, "/file.ext");

  f3d_test_check_int(&test, "engine_load_plugin(long name)", f3d_engine_load_plugin(long_name), 0);

  // f3d_engine_autoload_plugins tests
  f3d_engine_autoload_plugins();
  f3d_test_check(&test, "engine_autoload_plugins()", 1);

  // f3d_engine_get_plugins_list tests
  f3d_test_check_null(&test, "engine_get_plugins_list(NULL)", f3d_engine_get_plugins_list(NULL));

  char** plugins = f3d_engine_get_plugins_list("invalid_path");
  f3d_test_check_null(&test, "engine_get_plugins_list(invalid path)", plugins);

  plugins = f3d_engine_get_plugins_list(F3D_PLUGINS_INSTALL_DIR);
  if (plugins)
  {
    f3d_test_check_ptr(&test, "engine_get_plugins_list(valid)", plugins);
    f3d_test_check_ptr(&test, "engine_get_plugins_list first entry", plugins[0]);
    f3d_engine_free_string_array(plugins);
  }
  else
  {
    f3d_test_check(&test, "engine_get_plugins_list(no plugins installed)", 1);
  }



  //oldd
  const f3d_scene_t* scene = f3d_engine_get_scene(engine);
  f3d_test_check(&test, "get_scene() returns a valid scene", scene != NULL);

  const f3d_options_t* options = f3d_engine_get_options(engine);
  f3d_test_check(&test, "get_options() returns valid options", options != NULL);

  f3d_window_t* window = f3d_engine_get_window(engine);
  f3d_test_check(&test, "window retrieved from windowed engine", window != NULL);

  f3d_interactor_t* interactor = f3d_engine_get_interactor(engine);
  f3d_test_check(&test, "interactor retrieved from windowed engine", interactor != NULL);

  int cache_ret = f3d_engine_set_cache_path(engine, "/tmp/f3d_test_cache");
  f3d_test_check_int(&test, "set_cache_path succeeds for a valid path", cache_ret, 1);

  f3d_engine_autoload_plugins();
  
  int load_native = f3d_engine_load_plugin("native");
  f3d_test_check_int(&test, "loading the native plugin succeeds", load_native, 1);

  int load_invalid = f3d_engine_load_plugin("inexistent_plugin");
  f3d_test_check_int(&test, "loading a nonexistent plugin reports failure", load_invalid, 0);

  char** plugins_list = f3d_engine_get_plugins_list("inexistent_directory");
  f3d_test_check(&test, "plugins_list for nonexistent directory is NULL", plugins_list == NULL);

  int bad_option_ret = f3d_engine_set_reader_option("invalid.option", "value");
  f3d_test_check_int(&test, "setting an invalid reader option reports failure", bad_option_ret, 0);

  int skin_index_ret = f3d_engine_set_reader_option("QuakeMDL.skin_index", "0");
  f3d_test_check_int(&test, "setting QuakeMDL.skin_index to a valid value succeeds", skin_index_ret, 1);

  char** option_names = f3d_engine_get_all_reader_option_names();
  f3d_test_check(&test, "get_all_reader_option_names returns a non-null array", option_names != NULL);
  f3d_engine_free_string_array(option_names);

  f3d_backend_info_t* backends = f3d_engine_get_rendering_backend_list(NULL);
  f3d_test_check(&test, "get_rendering_backend_list returns a non-null array", backends != NULL);
  f3d_engine_free_backend_list(backends);

  f3d_lib_info_t* lib_info = f3d_engine_get_lib_info();
  f3d_test_check(&test, "get_lib_info() returns valid info", lib_info != NULL);
  if (lib_info)
  {
    f3d_test_check(
      &test, "lib_info has version and vtk_version", lib_info->version && lib_info->vtk_version);
    f3d_engine_free_lib_info(lib_info);
  }

  int reader_count = 0;
  f3d_reader_info_t* readers = f3d_engine_get_readers_info(&reader_count);
  f3d_test_check(
    &test, "get_readers_info() returns at least one reader", readers && reader_count > 0);
  f3d_engine_free_readers_info(readers);

  // set_options with the engine's own options has no distinct observable effect to check
  f3d_options_t* new_options = f3d_engine_get_options(engine);
  f3d_engine_set_options(engine, new_options);

  f3d_engine_t* engine_none = f3d_engine_create_none();
  f3d_test_check(&test, "engine_create_none() returns a valid engine", engine_none != NULL);
  if (engine_none)
  {
    f3d_engine_delete(engine_none);
  }

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}
