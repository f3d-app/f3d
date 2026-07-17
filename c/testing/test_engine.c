#include "pseudo_unit_test.h"
#include <engine_c_api.h>
#include <interactor_c_api.h>
#include <scene_c_api.h>
#include <window_c_api.h>
#include <options_c_api.h>
#include <string.h>

static void (*test_get_proc_address(const char* name))()
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
  f3d_test_check(&test, "engine_create_external(stub loader)", 1);

  if (engine)
  {
    f3d_engine_delete(engine);
  }

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

  // Recreate engine for remaining tests
  engine = f3d_engine_create(1);
  f3d_test_check_ptr(&test, "engine recreated", engine);

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

  // f3d_engine_dump_to_string tests
  f3d_test_check_null(&test, "engine_dump_to_string(NULL)", f3d_engine_dump_to_string(NULL));

  const char* dump = f3d_engine_dump_to_string(engine);
  f3d_test_check_ptr(&test, "engine_dump_to_string(valid)", dump);

  if (dump)
  {
    f3d_test_check(&test, "engine_dump_to_string(non-empty)", dump[0] != '\0');
  }

  // f3d_engine_dump_to_file tests
  f3d_test_check_int(&test, "engine_dump_to_file(NULL engine)",
    f3d_engine_dump_to_file(NULL, F3D_TESTING_TEMP_DIR "/dump.f3d"), 0);

  f3d_test_check_int(&test, "engine_dump_to_file(NULL path)",
    f3d_engine_dump_to_file(engine, NULL), 0);

  f3d_test_check_int(&test, "engine_dump_to_file(valid)",
    f3d_engine_dump_to_file(engine, F3D_TESTING_TEMP_DIR "/dump.f3d"), 1);

  f3d_test_check_int(&test, "engine_dump_to_file(invalid path)",
    f3d_engine_dump_to_file(engine, "/invalid/path/dump.f3d"), 0);

  // f3d_engine_dump_to_clipboard tests
  f3d_test_check_int(&test, "engine_dump_to_clipboard(NULL engine)",
    f3d_engine_dump_to_clipboard(NULL), 0);

  f3d_engine_dump_to_clipboard(engine);
  f3d_test_check(&test, "engine_dump_to_clipboard(valid) does not crash", 1);

  // f3d_engine_load_from_string tests
  f3d_test_check_int(&test, "engine_load_from_string(NULL engine)",
    f3d_engine_load_from_string(NULL, dump), 0);

  f3d_test_check_int(&test, "engine_load_from_string(NULL content)",
    f3d_engine_load_from_string(engine, NULL), 0);

  f3d_test_check_int(&test, "engine_load_from_string(invalid content)",
    f3d_engine_load_from_string(engine, "this is not valid json"), 0);

  if (dump)
  {
    f3d_test_check_int(&test, "engine_load_from_string(valid)",
      f3d_engine_load_from_string(engine, dump), 1);

    f3d_engine_free_string(dump);
  }

  // f3d_engine_load_from_file tests
  f3d_test_check_int(&test, "engine_load_from_file(NULL engine)",
    f3d_engine_load_from_file(NULL, F3D_TESTING_TEMP_DIR "/dump.f3d"), 0);

  f3d_test_check_int(&test, "engine_load_from_file(NULL path)",
    f3d_engine_load_from_file(engine, NULL), 0);

  f3d_test_check_int(&test, "engine_load_from_file(nonexistent file)",
    f3d_engine_load_from_file(engine, F3D_TESTING_TEMP_DIR "/nonexistent_dump.f3d"), 0);

  f3d_test_check_int(&test, "engine_load_from_file(valid)",
    f3d_engine_load_from_file(engine, F3D_TESTING_TEMP_DIR "/dump.f3d"), 1);

  // f3d_engine_load_from_clipboard tests
  f3d_test_check_int(&test, "engine_load_from_clipboard(NULL engine)",
    f3d_engine_load_from_clipboard(NULL), 0);

  f3d_engine_load_from_clipboard(engine);
  f3d_test_check(&test, "engine_load_from_clipboard(valid) does not crash", 1);

  // f3d_engine_free_string tests
  f3d_engine_free_string(NULL);
  f3d_test_check(&test, "engine_free_string(NULL)", 1);

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

  // f3d_engine_get_scene tests
  f3d_test_check_null(&test, "engine_get_scene(NULL)", f3d_engine_get_scene(NULL));

  f3d_test_check_ptr(&test, "engine_get_scene(valid)", f3d_engine_get_scene(engine));

  // f3d_engine_get_interactor tests
  f3d_test_check_null(&test, "engine_get_interactor(NULL)", f3d_engine_get_interactor(NULL));

  f3d_test_check_ptr(&test, "engine_get_interactor(valid)", f3d_engine_get_interactor(engine));

  f3d_test_check_null(
    &test, "engine_get_interactor(no interactor)", f3d_engine_get_interactor(none_engine));

  f3d_engine_delete(none_engine);

  // f3d_engine_get_rendering_backend_list / f3d_engine_free_backend_list tests
  int backend_count = -1;
  f3d_backend_info_t* backends = f3d_engine_get_rendering_backend_list(&backend_count);
  f3d_test_check_ptr(&test, "engine_get_rendering_backend_list(valid)", backends);
  f3d_test_check(&test, "engine_get_rendering_backend_list(count >= 0)", backend_count >= 0);

  if (backends)
  {
    int counted = 0;
    for (int i = 0; backends[i].name != NULL; i++)
    {
      counted++;
    }
    f3d_test_check_int(
      &test, "engine_get_rendering_backend_list(array length matches count)", counted, backend_count);
  }

  f3d_engine_free_backend_list(backends);
  f3d_test_check(&test, "engine_free_backend_list(valid)", 1);

  f3d_engine_free_backend_list(NULL);
  f3d_test_check(&test, "engine_free_backend_list(NULL)", 1);

  f3d_backend_info_t* backends_no_count = f3d_engine_get_rendering_backend_list(NULL);
  f3d_test_check_ptr(&test, "engine_get_rendering_backend_list(NULL count)", backends_no_count);
  f3d_engine_free_backend_list(backends_no_count);

  // f3d_engine_load_plugin tests
  f3d_test_check_int(&test, "engine_load_plugin(NULL)", f3d_engine_load_plugin(NULL), 0);

  f3d_test_check_int(
    &test, "engine_load_plugin(invalid library)", f3d_engine_load_plugin("invalid.so"), 0);

  char long_name[268];
  long_name[0] = '/';
  memset(long_name + 1, 'x', 257);
  strcpy(long_name + 258, "/file.ext");

  f3d_test_check_int(&test, "engine_load_plugin(long name)", f3d_engine_load_plugin(long_name), 0);

  // TODO: add engine_load_plugin(valid) using a known static plugin name 

  // f3d_engine_autoload_plugins tests
  f3d_engine_autoload_plugins();
  f3d_test_check(&test, "engine_autoload_plugins()", 1);

  // f3d_engine_get_plugins_list tests
  f3d_test_check_null(&test, "engine_get_plugins_list(NULL)", f3d_engine_get_plugins_list(NULL));

  char** plugins = f3d_engine_get_plugins_list("invalid_path");
  f3d_test_check_null(&test, "engine_get_plugins_list(invalid path)", plugins);

  // TODO: add engine_get_plugins_list(valid dir) using a known plugin json directory.

  // f3d_engine_get_all_reader_option_names / f3d_engine_free_string_array tests
  char** reader_option_names = f3d_engine_get_all_reader_option_names();
  f3d_test_check_ptr(&test, "engine_get_all_reader_option_names(valid)", reader_option_names);

  f3d_engine_free_string_array(reader_option_names);
  f3d_test_check(&test, "engine_free_string_array(valid)", 1);

  f3d_engine_free_string_array(NULL);
  f3d_test_check(&test, "engine_free_string_array(NULL)", 1);

  // f3d_engine_set_reader_option tests
  f3d_test_check_int(
    &test, "engine_set_reader_option(NULL name)", f3d_engine_set_reader_option(NULL, "1"), 0);

  f3d_test_check_int(&test, "engine_set_reader_option(NULL value)",
    f3d_engine_set_reader_option("some.option", NULL), 0);

  f3d_test_check_int(&test, "engine_set_reader_option(inexistent option)",
    f3d_engine_set_reader_option("nonexistent.option.name", "1"), 0);

  // TODO: add engine_set_reader_option(valid) using a known name/value pair

  // f3d_engine_get_lib_info / f3d_engine_free_lib_info tests
  f3d_lib_info_t* lib_info = f3d_engine_get_lib_info();
  f3d_test_check_ptr(&test, "engine_get_lib_info(valid)", lib_info);

  if (lib_info)
  {
    f3d_test_check_ptr(&test, "lib_info.version non-null", lib_info->version);
    if (lib_info->version)
    {
      f3d_test_check(&test, "lib_info.version non-empty", lib_info->version[0] != '\0');
    }
    f3d_test_check_ptr(&test, "lib_info.version_full non-null", lib_info->version_full);
    f3d_test_check_ptr(&test, "lib_info.build_date non-null", lib_info->build_date);
    f3d_test_check_ptr(&test, "lib_info.build_system non-null", lib_info->build_system);
    f3d_test_check_ptr(&test, "lib_info.compiler non-null", lib_info->compiler);
    f3d_test_check_ptr(&test, "lib_info.vtk_version non-null", lib_info->vtk_version);
    f3d_test_check_ptr(&test, "lib_info.license non-null", lib_info->license);
    f3d_test_check_ptr(&test, "lib_info.modules non-null", lib_info->modules);
    f3d_test_check_ptr(&test, "lib_info.copyrights non-null", lib_info->copyrights);
  }

  f3d_engine_free_lib_info(lib_info);
  f3d_test_check(&test, "engine_free_lib_info(valid)", 1);

  f3d_engine_free_lib_info(NULL);
  f3d_test_check(&test, "engine_free_lib_info(NULL)", 1);

  // f3d_engine_get_readers_info / f3d_engine_free_readers_info tests
  int reader_count = -1;
  f3d_reader_info_t* readers_info = f3d_engine_get_readers_info(&reader_count);
  f3d_test_check_ptr(&test, "engine_get_readers_info(valid)", readers_info);
  f3d_test_check(&test, "engine_get_readers_info(count >= 0)", reader_count >= 0);

  if (readers_info && reader_count > 0)
  {
    f3d_test_check_ptr(&test, "readers_info[0].name non-null", readers_info[0].name);
    f3d_test_check_ptr(&test, "readers_info[0].description non-null", readers_info[0].description);
    f3d_test_check_ptr(&test, "readers_info[0].extensions non-null", readers_info[0].extensions);
    f3d_test_check_ptr(&test, "readers_info[0].mime_types non-null", readers_info[0].mime_types);
    f3d_test_check_ptr(&test, "readers_info[0].plugin_name non-null", readers_info[0].plugin_name);
  }

  f3d_engine_free_readers_info(readers_info);
  f3d_test_check(&test, "engine_free_readers_info(valid)", 1);

  f3d_engine_free_readers_info(NULL);
  f3d_test_check(&test, "engine_free_readers_info(NULL)", 1);

  f3d_reader_info_t* readers_info_no_count = f3d_engine_get_readers_info(NULL);
  f3d_test_check_ptr(&test, "engine_get_readers_info(NULL count)", readers_info_no_count);
  f3d_engine_free_readers_info(readers_info_no_count);

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}