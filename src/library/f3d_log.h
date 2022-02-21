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
    appendArg(ss, args...);
    printInternal(sev, ss.str());
  }

  /**
   * If output window is a vtkF3DConsoleOutputWindow,
   * set the coloring usage.
   */
  static void setUseColoring(bool use);

  /**
   * Set if any log should be shown or not.
   */
  static void setQuiet(bool quiet);

  /**
   * If output window is a vtkF3DWin32OutputWindow,
   * this calls WaitForUser on the output window.
   * No effect otherwise.
   */
  static void waitForUser();

protected:
  static void appendArg(std::stringstream&) {}

  template<typename T, typename... Args>
  static void appendArg(std::stringstream& ss, T value, Args... args)
  {
    ss << value;
    appendArg(ss, args...);
  }

  static void printInternal(Severity sev, const std::string& msg);
};
}

#endif
