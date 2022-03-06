/**
 * @class   F3DLog
 * @brief   Namespace containing methods to print logs
 *
 */

#ifndef F3DLog_h
#define F3DLog_h

#include <string>

namespace F3DLog
{
enum class Severity : unsigned char
{
  Info,
  Warning,
  Error
};

/**
 * Print a message with corresponding severitry in the output window
 */
void Print(Severity sev, const std::string& msg);

/**
 * If output window is a vtkF3DConsoleOutputWindow,
 * set the coloring usage.
 */
void SetUseColoring(bool use);

/**
 * Set if any log should be shown or not.
 */
void SetQuiet(bool quiet);

/**
 * If output window is a vtkF3DWin32OutputWindow,
 * this calls WaitForUser on the output window.
 * No effect otherwise.
 */
void WaitForUser();
};

#endif
