/**
 * @class   levenshteinDistance
 * @brief   Compute the distance between two strings
 *
 * TODO
 */

#ifndef f3d_levenshteinDistance_h
#define f3d_levenshteinDistance_h

#include "export.h"

#include <string_view>
#include <memory>

namespace f3d
{
class F3D_EXPORT levenshteinDistance
{
public:
  levenshteinDistance(std::string_view a, std::string_view b);
  ~levenshteinDistance();

  size_t run();

  levenshteinDistance(levenshteinDistance const&) = delete;
  void operator=(levenshteinDistance const&) = delete;

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
#endif
