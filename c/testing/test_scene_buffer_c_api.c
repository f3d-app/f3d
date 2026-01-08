#include <engine_c_api.h>
#include <options_c_api.h>
#include <scene_c_api.h>

#include <stdio.h>
#include <string.h>

int test_scene_buffer_c_api()
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

  // Test adding a memory buffer

  char source[256];
  FILE *fp = fopen(F3D_TESTING_DATA_DIR "points.ply", "r");
  size_t readLength = 0;
  if (fp != NULL)
  {
    readLength = fread(source, sizeof(char), 256, fp);
    fclose(fp);
  }
  f3d_options_t* options = f3d_engine_get_options(engine);
  f3d_options_set_as_string(options, "scene.force_reader", "PLYReader");
  f3d_scene_add_buffer(scene, source, readLength);
  f3d_options_reset(options, "scene.force_reader");

  f3d_engine_delete(engine);
  fclose(fp);
  return 0;
}
