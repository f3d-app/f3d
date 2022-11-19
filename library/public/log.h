#ifndef f3d_log_h
#define f3d_log_h

#include "export.h"

#include <sstream>
#include <string>

namespace f3d
{
/**
 * @class   log
 * @brief   Class used to show logs in F3D
 *
 * A class to output logs to the standard output.
 * It supports different levels, errors, warnings and info, with associated coloring.
 * A few static methods exists to control the coloring and the verbosity level
 * A waitForUser utility static method exist for windows.
 *
 */
class F3D_EXPORT log
{
public:
  /**
   * Enumeration of verbose levels
   * =============================
   * DEBUG: All logs are displayed.
   * INFO: Standard logging level, the default.
   * WARN: Only warnings and errors are displayed.
   * ERROR: Only errors are displayed.
   * QUIET: Logging is fully disabled.
   */
  enum class VerboseLevel : unsigned char
  {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
    QUIET
  };

  /**
   * Log provided args as provided verbose level.
   */
  template<typename... Args>
  static void print(VerboseLevel level, Args... args)
  {
    std::stringstream ss;
    log::appendArg(ss, args...);
    log::printInternal(level, ss.str());
  }

  /**
   * Log provided args as a debug.
   */
  template<typename... Args>
  static void debug(Args... args)
  {
    std::stringstream ss;
    log::appendArg(ss, args...);
    log::debugInternal(ss.str());
  }

  /**
   * Log provided args as an info.
   */
  template<typename... Args>
  static void info(Args... args)
  {
    std::stringstream ss;
    log::appendArg(ss, args...);
    log::infoInternal(ss.str());
  }

  /**
   * Log provided args as a warning.
   */
  template<typename... Args>
  static void warn(Args... args)
  {
    std::stringstream ss;
    log::appendArg(ss, args...);
    log::warnInternal(ss.str());
  }

  /**
   * Log provided args as an error.
   */
  template<typename... Args>
  static void error(Args... args)
  {
    std::stringstream ss;
    log::appendArg(ss, args...);
    log::errorInternal(ss.str());
  }

  /**
   * Set the coloring usage, if applicable (eg: console output).
   */
  static void setUseColoring(bool use);

  /**
   * Set the verbose level.
   */
  static void setVerboseLevel(VerboseLevel level);

  /**
   * Wait for user if applicable (eg: win32 output window).
   * No effect otherwise.
   */
  static void waitForUser();

protected:
  //! @cond
  static void appendArg(std::stringstream&) {}

  template<typename T, typename... Args>
  static void appendArg(std::stringstream& ss, T value, Args... args)
  {
    ss << value;
    log::appendArg(ss, args...);
  }

  static void printInternal(VerboseLevel level, const std::string& msg);
  static void errorInternal(const std::string& msg);
  static void warnInternal(const std::string& msg);
  static void infoInternal(const std::string& msg);
  static void debugInternal(const std::string& msg);
  //! @endcond
};
}

#endif
