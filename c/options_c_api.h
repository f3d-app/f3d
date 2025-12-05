#ifndef F3D_OPTIONS_C_API_H
#define F3D_OPTIONS_C_API_H

#include "export.h"
#include "types_c_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct f3d_options_t f3d_options_t;

  ///@{ @name Options lifecycle
  /**
   * @brief Create a new options object.
   *
   * The returned options object must be freed with f3d_options_delete().
   *
   * @return Options handle.
   */
  F3D_EXPORT f3d_options_t* f3d_options_create();

  /**
   * @brief Delete an options object.
   *
   * @param options Options handle to delete.
   */
  F3D_EXPORT void f3d_options_delete(f3d_options_t* options);
  ///@}

  ///@{ @name Option setters
  /**
   * @brief Set an option value as a boolean.
   *
   * @param options Options handle.
   * @param name Option name.
   * @param value Boolean value (0 for false, non-zero for true).
   */
  F3D_EXPORT void f3d_options_set_as_bool(f3d_options_t* options, const char* name, int value);

  /**
   * @brief Set an option value as an integer.
   *
   * @param options Options handle.
   * @param name Option name.
   * @param value Integer value.
   */
  F3D_EXPORT void f3d_options_set_as_int(f3d_options_t* options, const char* name, int value);

  /**
   * @brief Set an option value as a double.
   *
   * @param options Options handle.
   * @param name Option name.
   * @param value Double value.
   */
  F3D_EXPORT void f3d_options_set_as_double(f3d_options_t* options, const char* name, double value);

  /**
   * @brief Set an option value as a string.
   *
   * @param options Options handle.
   * @param name Option name.
   * @param value String value.
   */
  F3D_EXPORT void f3d_options_set_as_string(
    f3d_options_t* options, const char* name, const char* value);

  /**
   * @brief Set an option value as a double vector.
   *
   * @param options Options handle.
   * @param name Option name.
   * @param values Array of double values.
   * @param count Number of values in the array.
   */
  F3D_EXPORT void f3d_options_set_as_double_vector(
    f3d_options_t* options, const char* name, const double* values, size_t count);

  /**
   * @brief Set an option value as an integer vector.
   *
   * @param options Options handle.
   * @param name Option name.
   * @param values Array of integer values.
   * @param count Number of values in the array.
   */
  F3D_EXPORT void f3d_options_set_as_int_vector(
    f3d_options_t* options, const char* name, const int* values, size_t count);
  ///@}

  ///@{ @name Option getters
  /**
   * @brief Get an option value as a boolean.
   *
   * @param options Options handle.
   * @param name Option name.
   * @return Boolean value (0 for false, non-zero for true).
   */
  F3D_EXPORT int f3d_options_get_as_bool(const f3d_options_t* options, const char* name);

  /**
   * @brief Get an option value as an integer.
   *
   * @param options Options handle.
   * @param name Option name.
   * @return Integer value.
   */
  F3D_EXPORT int f3d_options_get_as_int(const f3d_options_t* options, const char* name);

  /**
   * @brief Get an option value as a double.
   *
   * @param options Options handle.
   * @param name Option name.
   * @return Double value.
   */
  F3D_EXPORT double f3d_options_get_as_double(const f3d_options_t* options, const char* name);

  /**
   * @brief Get an option value as a string.
   *
   * The returned string is heap-allocated and must be freed with f3d_options_free_string().
   *
   * @param options Options handle.
   * @param name Option name.
   * @return String value.
   */
  F3D_EXPORT const char* f3d_options_get_as_string(const f3d_options_t* options, const char* name);

  /**
   * @brief Get an option value as a string representation.
   *
   * The returned string is heap-allocated and must be freed with f3d_options_free_string().
   *
   * @param options Options handle.
   * @param name Option name.
   * @return String representation of the option value.
   */
  F3D_EXPORT const char* f3d_options_get_as_string_representation(
    const f3d_options_t* options, const char* name);

  /**
   * @brief Set an option value from a string representation.
   *
   * Parses the string and sets the option to the appropriate type.
   *
   * @param options Options handle.
   * @param name Option name.
   * @param str String representation of the value.
   */
  F3D_EXPORT void f3d_options_set_as_string_representation(
    f3d_options_t* options, const char* name, const char* str);

  /**
   * @brief Free a string returned by an options function.
   *
   * Use this to free strings returned by f3d_options_get_as_string()
   * and f3d_options_get_as_string_representation().
   *
   * @param str String to free.
   */
  F3D_EXPORT void f3d_options_free_string(const char* str);

  /**
   * @brief Get an option value as a double vector.
   *
   * The caller must provide a pre-allocated array large enough to hold the values.
   *
   * @param options Options handle.
   * @param name Option name.
   * @param values Pre-allocated array to store the double values.
   * @param count Pointer to store the number of values retrieved.
   */
  F3D_EXPORT void f3d_options_get_as_double_vector(
    const f3d_options_t* options, const char* name, double* values, size_t* count);

  /**
   * @brief Get an option value as an integer vector.
   *
   * The caller must provide a pre-allocated array large enough to hold the values.
   *
   * @param options Options handle.
   * @param name Option name.
   * @param values Pre-allocated array to store the integer values.
   * @param count Pointer to store the number of values retrieved.
   */
  F3D_EXPORT void f3d_options_get_as_int_vector(
    const f3d_options_t* options, const char* name, int* values, size_t* count);
  ///@}

  ///@{ @name Option manipulation
  /**
   * @brief Toggle a boolean option value.
   *
   * @param options Options handle.
   * @param name Option name.
   */
  F3D_EXPORT void f3d_options_toggle(f3d_options_t* options, const char* name);

  /**
   * @brief Check if an option value is the same in two options objects.
   *
   * @param options Options handle.
   * @param other Other options handle to compare with.
   * @param name Option name.
   * @return 1 if the values are the same, 0 otherwise.
   */
  F3D_EXPORT int f3d_options_is_same(
    const f3d_options_t* options, const f3d_options_t* other, const char* name);

  /**
   * @brief Check if an option has a value set.
   *
   * @param options Options handle.
   * @param name Option name.
   * @return 1 if the option has a value, 0 otherwise.
   */
  F3D_EXPORT int f3d_options_has_value(const f3d_options_t* options, const char* name);

  /**
   * @brief Copy an option value from another options object.
   *
   * @param options Destination options handle.
   * @param other Source options handle to copy from.
   * @param name Option name.
   */
  F3D_EXPORT void f3d_options_copy(
    f3d_options_t* options, const f3d_options_t* other, const char* name);
  ///@}

  /**
   * @brief Get all option names.
   *
   * The returned array is heap-allocated and must be freed with f3d_options_free_names().
   *
   * @param count Pointer to store the count of names.
   * @return Array of option names.
   */
  F3D_EXPORT char** f3d_options_get_all_names(size_t* count);

  /**
   * @brief Get option names that have values.
   *
   * The returned array is heap-allocated and must be freed with f3d_options_free_names().
   *
   * @param options Options handle.
   * @param count Pointer to store the count of names.
   * @return Array of option names.
   */
  F3D_EXPORT char** f3d_options_get_names(const f3d_options_t* options, size_t* count);

  /**
   * @brief Get the closest option name and its Levenshtein distance.
   *
   * @param options Options handle.
   * @param option Option name to match.
   * @param closest Output parameter for the closest option name. Caller must free with
   *                f3d_options_free_string().
   * @param distance Output parameter for the Levenshtein distance.
   */
  F3D_EXPORT void f3d_options_get_closest_option(
    const f3d_options_t* options, const char* option, char** closest, unsigned int* distance);

  /**
   * @brief Free an array of option names.
   *
   * @param names Array of names to free.
   * @param count Number of names in the array.
   */
  F3D_EXPORT void f3d_options_free_names(char** names, size_t count);

  /**
   * @brief Check if an option is optional.
   *
   * @param options Options handle.
   * @param name Option name.
   * @return 1 if the option is optional, 0 otherwise.
   */
  F3D_EXPORT int f3d_options_is_optional(const f3d_options_t* options, const char* name);

  /**
   * @brief Reset an option to its default value.
   *
   * @param options Options handle.
   * @param name Option name.
   */
  F3D_EXPORT void f3d_options_reset(f3d_options_t* options, const char* name);

  /**
   * @brief Remove an option value if it is optional.
   *
   * @param options Options handle.
   * @param name Option name.
   */
  F3D_EXPORT void f3d_options_remove_value(f3d_options_t* options, const char* name);

  ///@{ @name Parsing and formatting
  /**
   * @brief Parse a string as a boolean.
   *
   * @param str String to parse.
   * @return 1 for true, 0 for false.
   */
  F3D_EXPORT int f3d_options_parse_bool(const char* str);

  /**
   * @brief Parse a string as an integer.
   *
   * @param str String to parse.
   * @return Parsed integer value.
   */
  F3D_EXPORT int f3d_options_parse_int(const char* str);

  /**
   * @brief Parse a string as a double.
   *
   * @param str String to parse.
   * @return Parsed double value.
   */
  F3D_EXPORT double f3d_options_parse_double(const char* str);

  /**
   * @brief Parse a string as a string (returns a copy).
   *
   * @param str String to parse.
   * @return Parsed string. Caller must free with f3d_options_free_string().
   */
  F3D_EXPORT const char* f3d_options_parse_string(const char* str);

  /**
   * @brief Parse a string as a double vector.
   *
   * @param str String to parse.
   * @param values Pre-allocated array to store the double values.
   * @param count Pointer to store the number of values retrieved.
   */
  F3D_EXPORT void f3d_options_parse_double_vector(const char* str, double* values, size_t* count);

  /**
   * @brief Parse a string as an integer vector.
   *
   * @param str String to parse.
   * @param values Pre-allocated array to store the integer values.
   * @param count Pointer to store the number of values retrieved.
   */
  F3D_EXPORT void f3d_options_parse_int_vector(const char* str, int* values, size_t* count);

  /**
   * @brief Format a boolean as a string.
   *
   * @param value Boolean value.
   * @return Formatted string. Caller must free with f3d_options_free_string().
   */
  F3D_EXPORT const char* f3d_options_format_bool(int value);

  /**
   * @brief Format an integer as a string.
   *
   * @param value Integer value.
   * @return Formatted string. Caller must free with f3d_options_free_string().
   */
  F3D_EXPORT const char* f3d_options_format_int(int value);

  /**
   * @brief Format a double as a string.
   *
   * @param value Double value.
   * @return Formatted string. Caller must free with f3d_options_free_string().
   */
  F3D_EXPORT const char* f3d_options_format_double(double value);

  /**
   * @brief Format a string (returns a copy).
   *
   * @param value String value.
   * @return Formatted string. Caller must free with f3d_options_free_string().
   */
  F3D_EXPORT const char* f3d_options_format_string(const char* value);

  /**
   * @brief Format a double vector as a string.
   *
   * @param values Array of double values.
   * @param count Number of values in the array.
   * @return Formatted string. Caller must free with f3d_options_free_string().
   */
  F3D_EXPORT const char* f3d_options_format_double_vector(const double* values, size_t count);

  /**
   * @brief Format an integer vector as a string.
   *
   * @param values Array of integer values.
   * @param count Number of values in the array.
   * @return Formatted string. Caller must free with f3d_options_free_string().
   */
  F3D_EXPORT const char* f3d_options_format_int_vector(const int* values, size_t count);
  ///@}

#ifdef __cplusplus
}
#endif

#endif // F3D_OPTIONS_C_API_H
