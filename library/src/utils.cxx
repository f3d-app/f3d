#include "utils.h"

#include "levenshtein.h"

namespace f3d
{
//----------------------------------------------------------------------------
unsigned int utils::textDistance(const std::string& strA, const std::string& strB)
{
  return static_cast<unsigned int>(levenshtein::distance(strA, strB));
}
}
