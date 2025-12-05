#include <engine_c_api.h>
#include <options_c_api.h>

#include <stdio.h>

int test_options_c_api()
{
  f3d_engine_t* engine = f3d_engine_create_none();
  if (!engine)
  {
    puts("[ERROR] Failed to create engine");
    return 1;
  }

  f3d_options_t* options = f3d_engine_get_options(engine);
  if (!options)
  {
    puts("[ERROR] Failed to get options");
    f3d_engine_delete(engine);
    return 1;
  }

  f3d_options_set_as_bool(options, "model.scivis.cells", 1);
  f3d_options_set_as_int(options, "model.scivis.component", 2);
  f3d_options_set_as_double(options, "render.line_width", 3.5);
  f3d_options_set_as_string(options, "render.effect.final_shader", "test.glsl");

  const double vec_values[] = { 1.0, 2.0, 3.0 };
  f3d_options_set_as_double_vector(options, "render.background.color", vec_values, 3);

  const int int_vec_values[] = { 1, 2 };
  f3d_options_set_as_int_vector(options, "scene.animation.indices", int_vec_values, 2);

  int bool_val = f3d_options_get_as_bool(options, "model.scivis.cells");
  (void)bool_val;

  int int_val = f3d_options_get_as_int(options, "model.scivis.component");
  (void)int_val;

  double double_val = f3d_options_get_as_double(options, "render.line_width");
  (void)double_val;

  const char* str_val = f3d_options_get_as_string(options, "render.effect.final_shader");
  if (str_val)
  {
    f3d_options_free_string(str_val);
  }

  double out_vec[3];
  size_t out_count;
  f3d_options_get_as_double_vector(options, "render.background.color", out_vec, &out_count);

  int out_int_vec[3];
  size_t out_int_count;
  f3d_options_get_as_int_vector(options, "scene.animation.indices", out_int_vec, &out_int_count);

  f3d_options_toggle(options, "model.scivis.cells");

  f3d_engine_t* engine2 = f3d_engine_create_none();
  if (engine2)
  {
    f3d_options_t* options2 = f3d_engine_get_options(engine2);
    if (options2)
    {
      int same = f3d_options_is_same(options, options2, "model.scivis.cells");
      (void)same;

      f3d_options_copy(options2, options, "model.scivis.cells");
    }
    f3d_engine_delete(engine2);
  }

  int has_value = f3d_options_has_value(options, "model.scivis.cells");
  (void)has_value;

  size_t all_count;
  char** all_names = f3d_options_get_all_names(&all_count);
  if (all_names)
  {
    f3d_options_free_names(all_names, all_count);
  }

  size_t names_count;
  char** names = f3d_options_get_names(options, &names_count);
  if (names)
  {
    f3d_options_free_names(names, names_count);
  }

  int is_optional = f3d_options_is_optional(options, "render.show_edges");
  (void)is_optional;

  f3d_options_reset(options, "model.scivis.cells");
  f3d_options_remove_value(options, "render.show_edges");

  const char* str_repr = f3d_options_get_as_string_representation(options, "render.line_width");
  if (str_repr)
  {
    f3d_options_free_string(str_repr);
  }

  f3d_options_set_as_string_representation(options, "render.line_width", "5.0");

  f3d_engine_delete(engine);
  return 0;
}
