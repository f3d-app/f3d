#include <utils_c_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_utils_c_api()
{
  int failed = 0;

  if (f3d_utils_text_distance("kitten", "sitting") != 3)
  {
    puts("[ERROR] text_distance(\"kitten\", \"sitting\") != 3");
    failed++;
  }

  if (f3d_utils_text_distance("same", "same") != 0)
  {
    puts("[ERROR] text_distance(\"same\", \"same\") != 0");
    failed++;
  }

  const char* cmd = "one two \"three four\" # comment";
  size_t tok_count = 0;
  char** tokens = f3d_utils_tokenize(cmd, 1, &tok_count);

  if (!(tokens && tok_count == 3 && strcmp(tokens[0], "one") == 0 &&
        strcmp(tokens[1], "two") == 0 && strcmp(tokens[2], "three four") == 0))
  {
    puts("[ERROR] tokenize() without comments failed");
    failed++;
  }

  f3d_utils_tokens_free(tokens, tok_count);

  char* collapsed = f3d_utils_collapse_path(".", NULL);
  if (!collapsed || collapsed[0] == '\0')
  {
    puts("[ERROR] collapse_path returned NULL/empty");
    failed++;
  }
  f3d_utils_string_free(collapsed);

  char* regex = f3d_utils_glob_to_regex("*.txt", '/');
  if (!regex || regex[0] == '\0')
  {
    puts("[ERROR] glob_to_regex returned NULL/empty");
    failed++;
  }
  f3d_utils_string_free(regex);

  char* env_val = f3d_utils_get_env("PATH");
  if (!env_val)
  {
    puts("[ERROR] get_env(\"PATH\") returned NULL");
    failed++;
  }
  f3d_utils_string_free(env_val);

#ifdef _WIN32
  char* kf = f3d_utils_get_known_folder(F3D_UTILS_KNOWN_FOLDER_ROAMINGAPPDATA);
  if (!kf || kf[0] == '\0')
  {
    puts("[ERROR] get_known_folder returned NULL/empty on Windows");
    failed++;
  }
  f3d_utils_string_free(kf);
#else
  char* kf = f3d_utils_get_known_folder(F3D_UTILS_KNOWN_FOLDER_ROAMINGAPPDATA);
  if (kf != NULL)
  {
    puts("[ERROR] get_known_folder should return NULL on non-Windows");
    f3d_utils_string_free(kf);
    failed++;
  }
#endif

  return failed;
}
