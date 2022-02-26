/**
 * @class   vtkF3DLog
 * @brief   Class used to show logs
 *
 */

#ifndef vtkF3DLog_h
#define vtkF3DLog_h

#include <vtkObject>

#include <sstream>
#include <string>

// TODO could this be a namespace instead of inheriting vtkObject ?
class vtkF3DLog : public vtkObject
{
public:
  enum class Severity : unsigned char
  {
    Info,
    Warning,
    Error
  };

  template<typename... Args>
  static void Print(Severity sev, Args... args)
  {
    std::stringstream ss;
    AppendArg(ss, args...);
    PrintInternal(sev, ss.str());
  }

  /**
   * If output window is a vtkF3DConsoleOutputWindow,
   * set the coloring usage.
   */
  static void SetUseColoring(bool use);

  /**
   * Set if any log should be shown or not.
   */
  static void SetQuiet(bool quiet);

  /**
   * If output window is a vtkF3DWin32OutputWindow,
   * this calls WaitForUser on the output window.
   * No effect otherwise.
   */
  static void WaitForUser();

protected:
  static void AppendArg(std::stringstream&) {}

  template<typename T, typename... Args>
  static void AppendArg(std::stringstream& ss, T value, Args... args)
  {
    ss << value;
    AppendArg(ss, args...);
  }

  static void PrintInternal(Severity sev, const std::string& msg);
};

#endif
