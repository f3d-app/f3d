#include "levenshtein.h"

#include <limits>
#include <vector>

namespace f3d
{
class levenshtein::internals
{
public:
  internals(const std::string_view& a, const std::string_view& b)
    : StringA(a)
    , StringB(b)
  {
    this->Distances.resize((a.size() + 1) * (b.size() + 1), std::numeric_limits<size_t>::max());
  }

  size_t run(size_t ia, size_t ib)
  {
    // check if this pair has already been checked in the cache and early exit
    if (this->getValue(ia, ib) != std::numeric_limits<size_t>::max())
    {
      return this->getValue(ia, ib);
    }

    size_t dist = 0;
    if (ib >= this->StringB.size())
    {
      // no more characters in string B, return the characters count in string A
      dist = this->StringA.size() - ia;
    }
    else if (ia >= this->StringA.size())
    {
      // no more characters in string A, return the characters count in string B
      dist = this->StringB.size() - ib;
    }
    else if (this->StringA[ia] == this->StringB[ib])
    {
      // same character, continue with the remaining characters
      dist = this->run(ia + 1, ib + 1);
    }
    else
    {
      // different character, increase the distance and check with advancing only A, only B, or both
      dist = 1 +
        std::min(std::min(this->run(ia, ib + 1), this->run(ia + 1, ib)), this->run(ia + 1, ib + 1));
    }

    // cache the value for later
    this->getValue(ia, ib) = dist;

    return dist;
  }

private:
  size_t& getValue(size_t ia, size_t ib)
  {
    return this->Distances[ia * (this->StringB.size() + 1) + ib];
  };

  std::vector<size_t> Distances;
  std::string_view StringA;
  std::string_view StringB;
};

//----------------------------------------------------------------------------
levenshtein::levenshtein(const std::string_view& a, const std::string_view& b)
  : Internals(std::make_unique<levenshtein::internals>(a, b))
{
}

//----------------------------------------------------------------------------
levenshtein::~levenshtein() = default;

//----------------------------------------------------------------------------
size_t levenshtein::run()
{
  return this->Internals->run(0, 0);
}
}
