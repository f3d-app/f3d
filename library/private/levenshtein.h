#ifndef f3d_levenshtein_h
#define f3d_levenshtein_h

#include <memory>
#include <string_view>

namespace f3d::detail
{
/**
 * Run the Levenshtein distance computation and return the result
 */
size_t levenshtein(const std::string_view& a, const std::string_view& b);
}
#endif
