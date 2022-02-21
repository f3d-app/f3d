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
  static void info(Args... args)
  {
    log::print(log::Severity::Info, args...);
  }

  /**
   * Log provided args as a warning.
   */
  template<typename... Args>
  static void warn(Args... args)
  {
    log::print(log::Severity::Warning, args...);
  }

  /**
   * Log provided args as an error.
   */
  template<typename... Args>
  static void error(Args... args)
  {
    log::print(log::Severity::Error, args...);
  }

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

protected:
  enum class Severity : unsigned char
  {
    Info,
    Warning,
    Error
  };

  template<typename... Args>
  static void print(Severity sev, Args... args)
  {
    std::stringstream ss;
    log::appendArg(ss, args...);
    log::printInternal(sev, ss.str());
  }

  static void appendArg(std::stringstream&) {}

  template<typename T, typename... Args>
  static void appendArg(std::stringstream& ss, T value, Args... args)
  {
    ss << value;
    log::appendArg(ss, args...);
  }

  static void printInternal(Severity sev, const std::string& msg);
};
}

#endif
