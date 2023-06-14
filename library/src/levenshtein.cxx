#include "levenshtein.h"

#include <limits>
#include <vector>

namespace f3d::levenshtein
{
class levenshtein_impl
{
public:
  levenshtein_impl(const std::string_view& strA, const std::string_view& strB)
    : StringA(strA)
    , StringB(strB)
  {
    this->Distances.resize((strA.size() + 1) * (strB.size() + 1), std::numeric_limits<size_t>::max());
  }

  size_t run(size_t indexA, size_t indexB)
  {
    // check if this pair has already been checked in the cache and early exit
    if (this->getValue(indexA, indexB) != std::numeric_limits<size_t>::max())
    {
      return this->getValue(indexA, indexB);
    }

    size_t dist = 0;
    if (indexB >= this->StringB.size())
    {
      // no more characters in string B, return the characters count in string A
      dist = this->StringA.size() - indexA;
    }
    else if (indexA >= this->StringA.size())
    {
      // no more characters in string A, return the characters count in string B
      dist = this->StringB.size() - indexB;
    }
    else if (this->StringA[indexA] == this->StringB[indexB])
    {
      // same character, continue with the remaining characters
      dist = this->run(indexA + 1, indexB + 1);
    }
    else
    {
      // different character, increase the distance and check with advancing only A, only B, or both
      dist = 1 +
        std::min(std::min(this->run(indexA, indexB + 1), this->run(indexA + 1, indexB)), this->run(indexA + 1, indexB + 1));
    }

    // cache the value for later
    this->getValue(indexA, indexB) = dist;

    return dist;
  }

private:
  size_t& getValue(size_t indexA, size_t indexB)
  {
    return this->Distances[indexA * (this->StringB.size() + 1) + indexB];
  };

  std::vector<size_t> Distances;
  std::string_view StringA;
  std::string_view StringB;
};

//----------------------------------------------------------------------------
size_t distance(const std::string_view& a, const std::string_view& b)
{
  return levenshtein_impl(a, b).run(0, 0);
}
}
