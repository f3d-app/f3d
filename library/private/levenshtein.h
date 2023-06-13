/**
 * @class   levenshtein
 * @brief   Compute the distance between two strings
 *
 * The Levenshtein distance between two words is the minimum number of single-character edits
 * (insertions, deletions or substitutions) required to change one word into the other.
 *
 * See https://en.wikipedia.org/wiki/Levenshtein_distance
 */

#ifndef f3d_levenshtein_h
#define f3d_levenshtein_h

#include <memory>
#include <string_view>

namespace f3d::levenshtein
{
  /**
   * Run the Levenshtein distance computation and return the result
   */
  size_t distance(const std::string_view& a, const std::string_view& b);
}
#endif
