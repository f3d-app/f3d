#ifndef F3DOperators_h
#define F3DOperators_h

#include <algorithm>
#include <iterator>
#include <locale>
#include <sstream>
#include <vector>

//----------------------------------------------------------------------------
// Make the istream_iterator works with comma
// Uses std coding style for consistency
class comma_ctype : public std::ctype<char>
{
  mask ctype_table[table_size];
public:
  comma_ctype(size_t refs = 0)
    : std::ctype<char>(&ctype_table[0], false, refs)
    {
      std::copy_n(classic_table(), table_size, ctype_table);
      unsigned char comma = ',';
      ctype_table[comma] = (mask)space;
    }
};

static std::locale comma_locale(std::locale::classic(), new comma_ctype);

//----------------------------------------------------------------------------
// Operator<< for easy copy of vector into string
template<typename T>
std::ostream& operator<< (std::ostream &stream, const std::vector<T>& vector)
{
  std::copy(vector.begin(), vector.end(), std::ostream_iterator<T>(stream, ","));
  return stream;
}

//----------------------------------------------------------------------------
// Operator<< for easy copy of string into vector
template<typename T>
std::istream& operator>> (std::istream &stream, std::vector<T>& vector)
{
  stream.imbue(comma_locale);
  std::copy(std::istream_iterator<T>(stream), std::istream_iterator<T>(), std::back_inserter(vector));
  return stream;
}

#endif
