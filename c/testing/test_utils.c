#include "pseudo_unit_test.h"

#include <utils_c_api.h>

#include <string.h>

int test_utils()
{
  f3d_test_t test;
  f3d_test_init(&test);

  f3d_test_check_int(
    &test, "text_distance(\"kitten\", \"sitting\") is 3", f3d_utils_text_distance("kitten", "sitting"), 3);

  f3d_test_check_int(
    &test, "text_distance(\"same\", \"same\") is 0", f3d_utils_text_distance("same", "same"), 0);

  const char* cmd = "one two \"three four\" # comment";
  size_t tok_count = 0;
  char** tokens = f3d_utils_tokenize(cmd, 1, &tok_count);

  int tokenize_ok = tokens && tok_count == 3 && strcmp(tokens[0], "one") == 0 &&
    strcmp(tokens[1], "two") == 0 && strcmp(tokens[2], "three four") == 0;
  f3d_test_check(&test, "tokenize() without comments splits correctly", tokenize_ok);

  f3d_utils_tokens_free(tokens, tok_count);

  char* collapsed = f3d_utils_collapse_path(".", NULL);
  f3d_test_check(&test, "collapse_path returns non-empty string", collapsed && collapsed[0] != '\0');
  f3d_utils_string_free(collapsed);

  char* regex = f3d_utils_glob_to_regex("*.txt", '/');
  f3d_test_check(&test, "glob_to_regex returns non-empty string", regex && regex[0] != '\0');
  f3d_utils_string_free(regex);

  char* env_val = f3d_utils_get_env("PATH");
  f3d_test_check(&test, "get_env(\"PATH\") returns non-null", env_val != NULL);
  f3d_utils_string_free(env_val);

#ifdef _WIN32
  char* kf = f3d_utils_get_known_folder(F3D_UTILS_KNOWN_FOLDER_ROAMINGAPPDATA);
  f3d_test_check(&test, "get_known_folder returns non-empty string on Windows", kf && kf[0] != '\0');
  f3d_utils_string_free(kf);
#else
  char* kf = f3d_utils_get_known_folder(F3D_UTILS_KNOWN_FOLDER_ROAMINGAPPDATA);
  f3d_test_check(&test, "get_known_folder returns NULL on non-Windows", kf == NULL);
  if (kf != NULL)
  {
    f3d_utils_string_free(kf);
  }
#endif

  return f3d_test_result(&test);
}
