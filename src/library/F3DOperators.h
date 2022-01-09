#ifndef F3DOperators_h
#define F3DOperators_h

#include <sstream>
#include <iterator>
#include <vector>

template<typename T>
std::ostream& operator<< (std::ostream &stream, const std::vector<T>& vector)
{
  std::copy(vector.begin(), vector.end(), std::ostream_iterator<T>(stream,"\n"));
  return stream;
}

template<typename T>
std::istream& operator>> (std::istream &stream, std::vector<T>& vector)
{
  copy(std::istream_iterator<T>(stream), std::istream_iterator<T>(), std::back_inserter(vector));
  return stream;
}

#endif
