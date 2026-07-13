#include "pseudo_unit_test.h"

#include <utils_c_api.h>

#include <string.h>

int test_utils()
{
  f3d_test_t test;
  f3d_test_init(&test);

  //f3d_utils_text_distance test cases
  f3d_test_check_int(
    &test, "text_distance(\"kitten\", \"sitting\") is 3", f3d_utils_text_distance("kitten", "sitting"), 3);
  
  f3d_test_check_int(
    &test, "text_distance(\"NULL\", \"NULL\") is 0", f3d_utils_text_distance(NULL, NULL), 0);

  f3d_test_check_int(
    &test, "text_distance(\"abc\", NULL) is 3", f3d_utils_text_distance("abc", NULL), 3);

  f3d_test_check_int(
    &test, "text_distance(NULL, \"abc\") is 3", f3d_utils_text_distance(NULL, "abc"), 3);

  //f3d_utils_tokenize test cases
  size_t count = 123;
  char** tokens = f3d_utils_tokenize(NULL, 0, &count);
  f3d_test_check_null(&test, "tokenize(NULL) returns NULL", tokens);
  f3d_test_check_size(&test, "tokenize(NULL) count is 0", count, 0);

  tokens = f3d_utils_tokenize(NULL, 0, NULL);
  f3d_test_check_null(&test, "tokenize(NULL, NULL count) returns NULL", tokens);

  count = 0;
  tokens = f3d_utils_tokenize("one two three", 0, &count);
  f3d_test_check_ptr(&test, "token array allocated", tokens);
  f3d_test_check_size(&test, "token count", count, 3);
  f3d_test_check_string(&test, "token[0]", tokens[0], "one");
  f3d_test_check_string(&test, "token[1]", tokens[1], "two");
  f3d_test_check_string(&test, "token[2]", tokens[2], "three");
  f3d_utils_tokens_free(tokens, count);

  tokens = f3d_utils_tokenize("one two", 0, NULL);
  f3d_test_check_ptr(&test, "NULL out_count still succeeds", tokens);
  f3d_test_check_string(&test, "token[0]", tokens[0], "one");
  f3d_test_check_string(&test, "token[1]", tokens[1], "two");
  f3d_utils_tokens_free(tokens, 2);

  count = 0;
  tokens = f3d_utils_tokenize("one two # comment", 1, &count);
  f3d_test_check_size(&test, "comments enabled: '#' starts a comment", count, 2);
  f3d_test_check_string(&test, "token[0]", tokens[0], "one");
  f3d_test_check_string(&test, "token[1]", tokens[1], "two");

  f3d_utils_tokens_free(tokens, count);
  count = 0;
  tokens = f3d_utils_tokenize("one two # comment", 0, &count);
  f3d_test_check_size(&test, "comments disabled: '#' treated as normal character", count, 4);
  f3d_test_check_string(&test, "token[0]", tokens[0], "one");
  f3d_test_check_string(&test, "token[1]", tokens[1], "two");
  f3d_test_check_string(&test, "token[2]", tokens[2], "#");
  f3d_test_check_string(&test, "token[3]", tokens[3], "comment");
  f3d_utils_tokens_free(tokens, count);

  count = 123;
  tokens = f3d_utils_tokenize("\"unterminated", 0, &count);
  f3d_test_check_null(&test, "unterminated quote returns NULL", tokens);
  f3d_test_check_size(&test, "unterminated quote count is 0", count, 0);

  count = 123;
  tokens = f3d_utils_tokenize("abc\\", 0, &count);
  f3d_test_check_null(&test, "trailing escape returns NULL", tokens);
  f3d_test_check_size(&test, "trailing escape count is 0", count, 0);
  
  //f3d_utils_collapse_path test cases
  char* path = f3d_utils_collapse_path("", NULL);
  f3d_test_check_string(&test, "collapse_path(empty)", path, "");
  f3d_utils_string_free(path); 

  path = f3d_utils_collapse_path("folder/file.ext", "/");
  f3d_test_check_string(&test, "collapse_path(relative, /)", path, "/folder/file.ext");
  f3d_utils_string_free(path);

  path = f3d_utils_collapse_path("/folder/../file.ext", NULL);
  f3d_test_check_string(&test, "collapse_path(remove dotdot)", path, "/file.ext");
  f3d_utils_string_free(path);

  //f3d_utils_glob_to_regex test cases
  char* regex = f3d_utils_glob_to_regex("*.txt", '/');
  f3d_test_check_ptr(&test, "glob_to_regex returned string", regex);
  f3d_test_check(&test, "glob_to_regex returned non-empty regex", regex[0] != '\0');
  f3d_utils_string_free(regex);

  regex = f3d_utils_glob_to_regex("file[0-9.txt", '/');
  f3d_test_check_null(&test, "glob_to_regex invalid character class returns NULL", regex);

  regex = f3d_utils_glob_to_regex("file1.txt\\", '/');
  f3d_test_check_null(&test, "glob_to_regex trailing escape returns NULL", regex);

  regex = f3d_utils_glob_to_regex("file.{txt,md", '/');
  f3d_test_check_null(&test, "glob_to_regex unmatched alternation returns NULL", regex);

  //f3d_utils_get_env test cases
  char* value = f3d_utils_get_env("F3D_TEST_ENV_STANDARD");
  f3d_test_check_ptr(&test, "get_env returned value", value);
  f3d_test_check_string(&test, "get_env standard", value, "TestEnv");
  f3d_utils_string_free(value);

  value = f3d_utils_get_env("F3D_TEST_ENV_EMPTY");
  f3d_test_check_null(&test, "get_env empty returns NULL", value);

  //f3d_utils_get_known_folder test cases
  char* folder = f3d_utils_get_known_folder((f3d_utils_known_folder_t)999);
  f3d_test_check_null(&test, "get_known_folder(invalid)", folder);

  #ifdef _WIN32

  folder = f3d_utils_get_known_folder(F3D_UTILS_KNOWN_FOLDER_ROAMINGAPPDATA);
  f3d_test_check_ptr(&test, "roaming appdata", folder);
  f3d_utils_string_free(folder);

  folder = f3d_utils_get_known_folder(F3D_UTILS_KNOWN_FOLDER_LOCALAPPDATA);
  f3d_test_check_ptr(&test, "local appdata", folder);
  f3d_utils_string_free(folder);

  folder = f3d_utils_get_known_folder(F3D_UTILS_KNOWN_FOLDER_PICTURES);
  f3d_test_check_ptr(&test, "pictures", folder);
  f3d_utils_string_free(folder);

  #else

  folder = f3d_utils_get_known_folder(F3D_UTILS_KNOWN_FOLDER_ROAMINGAPPDATA);
  f3d_test_check_null(&test, "roaming appdata unavailable", folder);

  #endif
  
  //f3d_utils_dpi_scale test cases
  double dpi_scale = f3d_utils_get_dpi_scale();
  f3d_test_check(&test, "get_dpi_scale returns positive value", dpi_scale > 0.0);

  return f3d_test_result(&test);
}
