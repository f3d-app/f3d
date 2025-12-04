#ifndef F3D_LOG_C_API_H
#define F3D_LOG_C_API_H

#include "export.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Enumeration of verbose levels.
   */
  typedef enum f3d_log_verbose_level_t
  {
    F3D_LOG_DEBUG = 0,
    F3D_LOG_INFO,
    F3D_LOG_WARN,
    F3D_LOG_ERROR,
    F3D_LOG_QUIET
  } f3d_log_verbose_level_t;

  /**
   * @brief Callback function type for log forwarding.
   *
   * @param level The verbose level of the log message.
   * @param message The log message string.
   */
  typedef void (*f3d_log_forward_fn_t)(f3d_log_verbose_level_t level, const char* message);

  /**
   * @brief Set the verbose level.
   *
   * By default, only warnings and errors are written to stderr, debug and info are written to
   * stdout. If force_std_err is non-zero, all messages including debug and info are written to
   * stderr.
   *
   * @param level The verbose level to set.
   * @param force_std_err If non-zero, all messages are written to stderr.
   */
  F3D_EXPORT void f3d_log_set_verbose_level(f3d_log_verbose_level_t level, int force_std_err);

  /**
   * @brief Get the current verbose level.
   *
   * @return The current verbose level.
   */
  F3D_EXPORT f3d_log_verbose_level_t f3d_log_get_verbose_level(void);

  /**
   * @brief Set the coloring usage, if applicable (e.g., console output).
   *
   * @param use If non-zero, coloring will be used.
   */
  F3D_EXPORT void f3d_log_set_use_coloring(int use);

  /**
   * @brief Log a message at the specified verbose level.
   *
   * @param level The verbose level for the message.
   * @param message The message string.
   */
  F3D_EXPORT void f3d_log_print(f3d_log_verbose_level_t level, const char* message);

  /**
   * @brief Set a callback function to forward log messages.
   *
   * The callback will be invoked with the level and the message string whenever a message is
   * logged, regardless of the verbose level. Set to NULL to disable forwarding.
   *
   * @param callback The callback function, or NULL to disable forwarding.
   */
  F3D_EXPORT void f3d_log_forward(f3d_log_forward_fn_t callback);

#ifdef __cplusplus
}
#endif

#endif // F3D_LOG_C_API_H
