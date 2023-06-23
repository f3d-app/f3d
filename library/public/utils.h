#ifndef f3d_utils_h
#define f3d_utils_h

#include "export.h"

#include <string>

namespace f3d
{
/**
 * @class   utils
 * @brief   Class containing useful static functions
 *
 * A class containing useful static functions unrelated to other classes.
 *
 */
class F3D_EXPORT utils
{
public:
  /**
   * Compute the Levenshtein distance between two strings.
   * Can be useful for spell checking and typo detection.
   */
  static unsigned int textDistance(const std::string& strA, const std::string& strB);
};
}

#endif
