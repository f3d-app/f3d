#include "levenshteinDistance.h"

#include <vector>

namespace f3d
{
class levenshteinDistance::internals
{
public:
  internals(std::string_view a, std::string_view b)
    : StringA(a)
    , StringB(b)
  {
    this->Distances.resize((a.size() + 1) * (b.size() + 1), -1);
  }

  size_t run(size_t ia, size_t ib)
  {
    if (this->getValue(ia, ib) != -1)
    {
      return this->getValue(ia, ib);
    }

    size_t dist = 0;
    if (ib >= this->StringB.size())
    {
      dist = this->StringA.size() - ia;
    }
    else if (ia >= this->StringA.size())
    {
      dist = this->StringB.size() - ib;
    }
    else if (this->StringA[ia] == this->StringB[ib])
    {
      dist = this->run(ia + 1, ib + 1);
    }
    else
    {
      dist = 1 +
        std::min(std::min(this->run(ia, ib + 1), this->run(ia + 1, ib)), this->run(ia + 1, ib + 1));
    }
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
levenshteinDistance::levenshteinDistance(std::string_view a, std::string_view b)
  : Internals(std::make_unique<levenshteinDistance::internals>(a, b))
{
}

//----------------------------------------------------------------------------
levenshteinDistance::~levenshteinDistance() = default;

//----------------------------------------------------------------------------
size_t levenshteinDistance::run()
{
  return this->Internals->run(0, 0);
}
}
