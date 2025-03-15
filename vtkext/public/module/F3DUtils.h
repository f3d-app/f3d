/**
 * @class   F3DUtils
 * @brief   Namespace containing utility methods for plugins
 *
 * Provide simple utilities to for plugins to avoid code duplication.
 */

#ifndef F3DUtils_h
#define F3DUtils_h

#include "vtkextModule.h"

#include <string>

namespace F3DUtils
{
/*
 * Convert provided std into a double and returns it.
 * Catch conversion error, log them if any and returns the provided def value.
 * Use nameError in the log for easier debugging.
 */
VTKEXT_EXPORT double ParseToDouble(
  const std::string& str, double def, const std::string& nameError);
};

#endif
