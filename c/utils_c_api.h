#ifndef F3D_UTILS_C_API_H
#define F3D_UTILS_C_API_H

#include "export.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Enumeration of supported Windows known folders.
   */
  typedef enum f3d_utils_known_folder_t
  {
    F3D_UTILS_KNOWN_FOLDER_ROAMINGAPPDATA,
    F3D_UTILS_KNOWN_FOLDER_LOCALAPPDATA,
    F3D_UTILS_KNOWN_FOLDER_PICTURES
  } f3d_utils_known_folder_t;

  /**
   * @brief Compute the Levenshtein distance between two strings.
   *
   * @param str_a First string.
   * @param str_b Second string.
   * @return The Levenshtein distance between the two strings.
   */
  F3D_EXPORT unsigned int f3d_utils_text_distance(const char* str_a, const char* str_b);

  /**
   * @brief Tokenize a string using the same logic as bash.
   *
   * The returned array and strings are heap-allocated and must be freed by calling
   * f3d_utils_tokens_free().
   *
   * @param str Input string to tokenize.
   * @param keep_comments Non-zero to keep comments, zero to treat '#' as a normal character.
   * @param out_count Pointer to receive the number of tokens.
   * @return Array of C strings.
   */
  F3D_EXPORT char** f3d_utils_tokenize(const char* str, int keep_comments, size_t* out_count);

  /**
   * @brief Free an array of tokens allocated by f3d_utils_tokenize().
   *
   * @param tokens Array of tokens.
   * @param count Number of tokens in the array.
   */
  F3D_EXPORT void f3d_utils_tokens_free(char** tokens, size_t count);

  /**
   * @brief Collapse a filesystem path.
   *
   * Expands '~' to the home directory, makes the path absolute using base_directory
   * or the current directory, and normalizes '..' components.
   *
   * The returned string is heap-allocated and must be freed with f3d_utils_string_free().
   *
   * @param path Input path.
   * @param base_directory Base directory for relative paths.
   * @return Collapsed absolute path string.
   */
  F3D_EXPORT char* f3d_utils_collapse_path(const char* path, const char* base_directory);

  /**
   * @brief Converts a glob expression to a regular expression.
   *
   * The returned string is heap-allocated and must be freed with f3d_utils_string_free().
   *
   * @param glob Glob expression.
   * @param path_separator Path separator character.
   * @return Regular expression string.
   */
  F3D_EXPORT char* f3d_utils_glob_to_regex(const char* glob, char path_separator);

  /**
   * @brief Get the value of an environment variable.
   *
   * The returned string is heap-allocated and must be freed with f3d_utils_string_free().
   *
   * @param env Environment variable name.
   * @return Value of the environment variable.
   */
  F3D_EXPORT char* f3d_utils_get_env(const char* env);

  /**
   * @brief Get a Windows known folder.
   *
   * The returned string is heap-allocated and must be freed with f3d_utils_string_free().
   *
   * @param known_folder Known folder identifier.
   * @return Folder path.
   */
  F3D_EXPORT char* f3d_utils_get_known_folder(f3d_utils_known_folder_t known_folder);

  /**
   * @brief Free a string returned by any f3d_utils_* function.
   *
   * @param str String to free.
   */
  F3D_EXPORT void f3d_utils_string_free(char* str);

  /**
   * @brief Calculate the primary monitor system zoom scale base on DPI.
   *
   * Only supported on Windows platform.
   *
   * @return DPI scale in double, or 1.0 on other platforms.
   */
  F3D_EXPORT double f3d_utils_get_dpi_scale();

#ifdef __cplusplus
}
#endif

#endif // F3D_UTILS_C_API_H
