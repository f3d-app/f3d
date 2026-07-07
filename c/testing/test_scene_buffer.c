#include "pseudo_unit_test.h"

#include <engine_c_api.h>
#include <options_c_api.h>
#include <scene_c_api.h>

#include <stdio.h>

int test_scene_buffer()
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

  char source[256];
  FILE* fp = fopen(F3D_TESTING_DATA_DIR "points.ply", "r");
  f3d_test_check(&test, "test data file opened", fp != NULL);

  size_t readLength = 0;
  if (fp != NULL)
  {
    readLength = fread(source, sizeof(char), 256, fp);
    fclose(fp);
  }
  f3d_test_check(&test, "test data file read some bytes", readLength > 0);

  f3d_options_t* options = f3d_engine_get_options(engine);
  f3d_options_set_as_string(options, "scene.force_reader", "PLYReader");

  int add_buffer_result = f3d_scene_add_buffer(scene, source, readLength);
  f3d_test_check_int(&test, "add_buffer with valid PLY data returns 1 (success)", add_buffer_result, 1);

  f3d_options_reset(options, "scene.force_reader");

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}