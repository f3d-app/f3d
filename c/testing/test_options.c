#include "pseudo_unit_test.h"

#include <engine_c_api.h>
#include <options_c_api.h>

#include <string.h>

int test_options()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_options_t* standalone_options = f3d_options_create();
  f3d_test_check(&test, "standalone options created", standalone_options != NULL);
  f3d_options_delete(standalone_options);

  f3d_engine_t* engine = f3d_engine_create_none();
  f3d_test_check(&test, "engine created", engine != NULL);
  if (!engine)
  {
    return f3d_test_result(&test);
  }

  f3d_options_t* options = f3d_engine_get_options(engine);
  f3d_test_check(&test, "options retrieved", options != NULL);
  if (!options)
  {
    f3d_engine_delete(engine);
    return f3d_test_result(&test);
  }

  /* direct set/get round trips: deterministic, safe to assert exactly */
  f3d_options_set_as_bool(options, "model.scivis.cells", 1);
  int bool_val = f3d_options_get_as_bool(options, "model.scivis.cells");
  f3d_test_check_int(&test, "bool value matches", bool_val, 1);

  f3d_options_set_as_int(options, "model.scivis.component", 2);
  int int_val = f3d_options_get_as_int(options, "model.scivis.component");
  f3d_test_check_int(&test, "int value matches", int_val, 2);

  f3d_options_set_as_double(options, "render.line_width", 3.5);
  double double_val = f3d_options_get_as_double(options, "render.line_width");
  f3d_test_check_double(&test, "double value matches", double_val, 3.5, 1e-9);

  f3d_options_set_as_string(options, "render.effect.final_shader", "test.glsl");
  const char* str_val = f3d_options_get_as_string(options, "render.effect.final_shader");
  f3d_test_check(
    &test, "string value matches", str_val != NULL && strcmp(str_val, "test.glsl") == 0);
  if (str_val)
  {
    f3d_options_free_string(str_val);
  }

  const double vec_values[] = { 1.0, 2.0, 3.0 };
  f3d_options_set_as_double_vector(options, "render.background.color", vec_values, 3);
  double out_vec[3];
  size_t out_count;
  f3d_options_get_as_double_vector(options, "render.background.color", out_vec, &out_count);
  f3d_test_check_int(&test, "double vector count matches", (long)out_count, 3);
  f3d_test_check_double(&test, "double vector[0] matches", out_vec[0], vec_values[0], 1e-9);
  f3d_test_check_double(&test, "double vector[1] matches", out_vec[1], vec_values[1], 1e-9);
  f3d_test_check_double(&test, "double vector[2] matches", out_vec[2], vec_values[2], 1e-9);

  const int int_vec_values[] = { 1, 2 };
  f3d_options_set_as_int_vector(options, "scene.animation.indices", int_vec_values, 2);
  int out_int_vec[3];
  size_t out_int_count;
  f3d_options_get_as_int_vector(options, "scene.animation.indices", out_int_vec, &out_int_count);
  f3d_test_check_int(&test, "int vector count matches", (long)out_int_count, 2);
  f3d_test_check_int(&test, "int vector[0] matches", out_int_vec[0], int_vec_values[0]);
  f3d_test_check_int(&test, "int vector[1] matches", out_int_vec[1], int_vec_values[1]);

  f3d_options_toggle(options, "model.scivis.cells");
  int toggled_val = f3d_options_get_as_bool(options, "model.scivis.cells");
  f3d_test_check_int(&test, "toggle flips bool value back to false", toggled_val, 0);

  f3d_engine_t* engine2 = f3d_engine_create_none();
  f3d_test_check(&test, "second engine created", engine2 != NULL);
  if (engine2)
  {
    f3d_options_t* options2 = f3d_engine_get_options(engine2);
    f3d_test_check(&test, "second options retrieved", options2 != NULL);
    if (options2)
    {
      int same = f3d_options_is_same(options, options2, "model.scivis.cells");
      f3d_test_check_int(&test, "is_same true when both at default false", same, 1);

      f3d_options_copy(options2, options, "model.scivis.cells");
      int copied_val = f3d_options_get_as_bool(options2, "model.scivis.cells");
      f3d_test_check_int(&test, "copy transfers value correctly", copied_val, toggled_val);
    }
    f3d_engine_delete(engine2);
  }

  int has_value = f3d_options_has_value(options, "model.scivis.cells");
  f3d_test_check(&test, "has_value true for a set option", has_value != 0);

  size_t all_count;
  char** all_names = f3d_options_get_all_names(&all_count);
  f3d_test_check(&test, "get_all_names returns names", all_names != NULL && all_count > 0);
  if (all_names)
  {
    f3d_options_free_names(all_names, all_count);
  }

  size_t names_count;
  char** names = f3d_options_get_names(options, &names_count);
  f3d_test_check(&test, "get_names returns names", names != NULL && names_count > 0);
  if (names)
  {
    f3d_options_free_names(names, names_count);
  }

  int is_optional_array_name = f3d_options_is_optional(options, "model.scivis.array_name");
  f3d_test_check_int(&test, "array_name is optional", is_optional_array_name, 1);

  int is_optional_cells = f3d_options_is_optional(options, "model.scivis.cells");
  f3d_test_check_int(&test, "cells is not optional", is_optional_cells, 0);

  /* render.show_edges has type "bool" in options.json -> valid, non-invalid type */
  f3d_option_type_t type = f3d_options_get_type(options, "render.show_edges");
  f3d_test_check_int(&test, "render.show_edges type is BOOL", (int)type, (int)F3D_OPTION_TYPE_BOOL);

  f3d_test_check(&test, "unknown option type is INVALID",
    f3d_options_get_type(options, "dummy") == F3D_OPTION_TYPE_INVALID);

  f3d_options_reset(options, "model.scivis.cells");
  int reset_val = f3d_options_get_as_bool(options, "model.scivis.cells");
  f3d_test_check_int(&test, "reset restores default value (false) for model.scivis.cells", reset_val, 0);

  f3d_options_remove_value(options, "render.show_edges");
  int has_value_after_remove = f3d_options_has_value(options, "render.show_edges");
  f3d_test_check(&test, "has_value false after remove_value", has_value_after_remove == 0);


  int has_domain = f3d_options_has_domain(options, "scene.animation.speed_factor");
  f3d_test_check_int(&test, "speed_factor has a domain", has_domain, 1);

  f3d_domain_style_t domain_style = f3d_options_get_domain_style(options, "scene.animation.speed_factor");
  f3d_test_check_int(&test, "speed_factor domain style is RANGE", (int)domain_style, (int)F3D_DOMAIN_STYLE_RANGE);

  int enum_count = 0;
  char** enumeration = f3d_options_get_enum_domain(options, "interactor.style", &enum_count);
  f3d_test_check(&test, "enum domain returns values", enumeration != NULL && enum_count > 0);
  if (enumeration)
  {
    f3d_options_free_string_array(enumeration, enum_count);
  }

  //smoke calls only, no observable state change to assert on
  f3d_options_increase(options, "scene.animation.speed_factor");
  f3d_options_decrease(options, "scene.animation.speed_factor");
  f3d_options_cycle(options, "interactor.style");

  const char* str_repr = f3d_options_get_as_string_representation(options, "render.line_width");
  f3d_test_check(&test, "string representation is non-null", str_repr != NULL);
  if (str_repr)
  {
    f3d_options_free_string(str_repr);
  }

  f3d_options_set_as_string_representation(options, "render.line_width", "5.0");
  double after_repr_set = f3d_options_get_as_double(options, "render.line_width");
  f3d_test_check_double(
    &test, "set_as_string_representation updates value", after_repr_set, 5.0, 1e-9);

  char* closest = NULL;
  unsigned int distance = 0;
  f3d_options_get_closest_option(options, "render.line_wdth", &closest, &distance);
  f3d_test_check(&test, "closest option suggestion is render.line_width",
    closest != NULL && strcmp(closest, "render.line_width") == 0);
  if (closest)
  {
    f3d_options_free_string(closest);
  }

  /* parsing helpers: pure, deterministic conversions on fixed input strings
   * we control, safe to assert exactly */
  int parsed_bool = f3d_options_parse_bool("true");
  f3d_test_check_int(&test, "parse_bool(\"true\") is true", parsed_bool, 1);

  int parsed_int = f3d_options_parse_int("42");
  f3d_test_check_int(&test, "parse_int(\"42\") matches", parsed_int, 42);

  double parsed_double = f3d_options_parse_double("3.14");
  f3d_test_check_double(&test, "parse_double(\"3.14\") matches", parsed_double, 3.14, 1e-9);

  const char* parsed_string = f3d_options_parse_string("test");
  f3d_test_check(&test, "parse_string(\"test\") matches",
    parsed_string != NULL && strcmp(parsed_string, "test") == 0);
  if (parsed_string)
  {
    f3d_options_free_string(parsed_string);
  }

  double parsed_dvec[3];
  size_t parsed_dvec_count;
  f3d_options_parse_double_vector("1.0,2.0,3.0", parsed_dvec, &parsed_dvec_count);
  f3d_test_check_int(&test, "parsed double vector count matches", (long)parsed_dvec_count, 3);
  f3d_test_check_double(&test, "parsed double vector[0] matches", parsed_dvec[0], 1.0, 1e-9);
  f3d_test_check_double(&test, "parsed double vector[1] matches", parsed_dvec[1], 2.0, 1e-9);
  f3d_test_check_double(&test, "parsed double vector[2] matches", parsed_dvec[2], 3.0, 1e-9);

  int parsed_ivec[3];
  size_t parsed_ivec_count;
  f3d_options_parse_int_vector("1,2,3", parsed_ivec, &parsed_ivec_count);
  f3d_test_check_int(&test, "parsed int vector count matches", (long)parsed_ivec_count, 3);
  f3d_test_check_int(&test, "parsed int vector[0] matches", parsed_ivec[0], 1);
  f3d_test_check_int(&test, "parsed int vector[1] matches", parsed_ivec[1], 2);
  f3d_test_check_int(&test, "parsed int vector[2] matches", parsed_ivec[2], 3);

  const char* fmt_bool = f3d_options_format_bool(1);
  f3d_test_check(
    &test, "format_bool(1) is \"true\"", fmt_bool != NULL && strcmp(fmt_bool, "true") == 0);
  if (fmt_bool)
  {
    f3d_options_free_string(fmt_bool);
  }

  const char* fmt_int = f3d_options_format_int(42);
  f3d_test_check(&test, "format_int(42) matches", fmt_int != NULL && strcmp(fmt_int, "42") == 0);
  if (fmt_int)
  {
    f3d_options_free_string(fmt_int);
  }

  const char* fmt_double = f3d_options_format_double(3.14);
  f3d_test_check(
    &test, "format_double(3.14) is \"3.14\"", fmt_double != NULL && strcmp(fmt_double, "3.14") == 0);
  if (fmt_double)
  {
    f3d_options_free_string(fmt_double);
  }

  const char* fmt_string = f3d_options_format_string("test");
  f3d_test_check(
    &test, "format_string(\"test\") matches", fmt_string != NULL && strcmp(fmt_string, "test") == 0);
  if (fmt_string)
  {
    f3d_options_free_string(fmt_string);
  }

  const double fmt_dvec[] = { 1.0, 2.0, 3.0 };
  const char* fmt_dvec_str = f3d_options_format_double_vector(fmt_dvec, 3);
  f3d_test_check(&test, "format_double_vector returns a string", fmt_dvec_str != NULL);
  if (fmt_dvec_str)
  {
    f3d_options_free_string(fmt_dvec_str);
  }

  const int fmt_ivec[] = { 1, 2, 3 };
  const char* fmt_ivec_str = f3d_options_format_int_vector(fmt_ivec, 3);
  f3d_test_check(&test, "format_int_vector returns a string", fmt_ivec_str != NULL);
  if (fmt_ivec_str)
  {
    f3d_options_free_string(fmt_ivec_str);
  }

  f3d_engine_delete(engine);
  return f3d_test_result(&test);
}
