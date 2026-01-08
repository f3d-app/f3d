/**
 * @class   F3DUtils
 * @brief   Namespace containing utility methods for plugins
 *
 * Provide simple utilities to for plugins to avoid code duplication.
 */

#ifndef F3DUtils_h
#define F3DUtils_h

#include "vtkWindow.h"
#include "vtkextModule.h"

/// @cond
#include <string>
/// @endcond

namespace F3DUtils
{
/*
 * Convert provided std into a double and returns it.
 * Catch conversion error, log them if any and returns the provided def value.
 * Use nameError in the log for easier debugging.
 */
VTKEXT_EXPORT double ParseToDouble(
  const std::string& str, double def, const std::string& nameError);

/*
 * Convert provided std into an int and returns it.
 * Catch conversion error, log them if any and returns the provided def value.
 * Use nameError in the log for easier debugging.
 */
VTKEXT_EXPORT int ParseToInt(const std::string& str, int def, const std::string& nameError);

/*
 * Get the monitor system scale base on DPI.
 * Supported on Windows and Linux.
 * Return a hardcoded 1.0 double on other platforms.
 *
 * Capability:
 *
 *    Windows:
 *        Handle DPI change at run-time:            Yes
 *        Handle DPI difference between monitor:    Yes
 *
 *    Linux:
 *        Handle DPI change at run-time:            No
 *        Handle DPI difference between monitor:    No
 */
VTKEXT_EXPORT double getDPIScale(vtkWindow* win = nullptr);
};

#endif
