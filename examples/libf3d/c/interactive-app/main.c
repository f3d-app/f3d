#include <f3d/engine_c_api.h>
#include <f3d/interactor_c_api.h>
#include <f3d/options_c_api.h>
#include <f3d/scene_c_api.h>
#include <f3d/window_c_api.h>

#include <stdio.h>
#include <stdlib.h>

static void timeout_callback(void* user_data)
{
  f3d_interactor_t* interactor = user_data;
  f3d_interactor_stop(interactor);
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char* file = argv[1];

  f3d_engine_autoload_plugins();

  f3d_engine_t* engine = f3d_engine_create(0);
  if (!engine)
  {
    fprintf(stderr, "Failed to create engine\n");
    return EXIT_FAILURE;
  }

  f3d_options_t* options = f3d_engine_get_options(engine);
  if (!options)
  {
    fprintf(stderr, "Failed to get options\n");
    f3d_engine_delete(engine);
    return EXIT_FAILURE;
  }

  f3d_options_set_as_bool(options, "render.grid.enable", 1);
  f3d_options_set_as_bool(options, "render.show_edges", 1);
  f3d_options_set_as_bool(options, "ui.axis", 1);
  f3d_options_set_as_bool(options, "ui.fps", 1);
  f3d_options_set_as_bool(options, "ui.animation_progress", 1);
  f3d_options_set_as_bool(options, "ui.filename", 1);

  f3d_scene_t* scene = f3d_engine_get_scene(engine);
  if (!scene)
  {
    fprintf(stderr, "Failed to get scene\n");
    f3d_engine_delete(engine);
    return EXIT_FAILURE;
  }

  if (!f3d_scene_add(scene, file))
  {
    fprintf(stderr, "Failed to load file: %s\n", file);
    f3d_engine_delete(engine);
    return EXIT_FAILURE;
  }

  f3d_window_t* window = f3d_engine_get_window(engine);
  if (!window)
  {
    fprintf(stderr, "Failed to get window\n");
    f3d_engine_delete(engine);
    return EXIT_FAILURE;
  }

  f3d_window_render(window);

  f3d_interactor_t* interactor = f3d_engine_get_interactor(engine);
  if (!interactor)
  {
    fprintf(stderr, "Failed to get interactor\n");
    f3d_engine_delete(engine);
    return EXIT_FAILURE;
  }

  if (argc > 2)
  {
    int timeout = atoi(argv[2]);
    f3d_interactor_start_with_callback(interactor, timeout, timeout_callback, interactor);
  }
  else
  {
    f3d_interactor_start(interactor, 1.0 / 30.0);
  }

  f3d_engine_delete(engine);

  return EXIT_SUCCESS;
}
