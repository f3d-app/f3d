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

namespace f3d
{
class levenshtein
{
public:
  levenshtein(std::string_view a, std::string_view b);
  ~levenshtein();

  /**
   * Run the Levenshtein distance computation and return the result
   */
  size_t run();

  levenshtein(levenshtein const&) = delete;
  void operator=(levenshtein const&) = delete;

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}
#endif
