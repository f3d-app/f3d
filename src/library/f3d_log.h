/**
 * @class   log
 * @brief   Class used to show logs in F3D
 *
 * TODO improve doc
 */

#ifndef f3d_log_h
#define f3d_log_h

#include <sstream>
#include <string>

namespace f3d
{
class log
{
public:
  /**
   * Log provided args as an info.
   */
  template<typename... Args>
  static void info(Args... args);

  /**
   * Log provided args as a warning.
   */
  template<typename... Args>
  static void warn(Args... args);

  /**
   * Log provided args as an error.
   */
  template<typename... Args>
  static void error(Args... args);

  /**
   * Set the coloring usage, if applicable (eg: console output).
   */
  static void setUseColoring(bool use);

  /**
   * Set if any log should be shown or not.
   */
  static void setQuiet(bool quiet);

  /**
   * Wait for user if applicable (eg: win32 output window)
   * No effect otherwise.
   */
  static void waitForUser();
};
}

#endif
