#ifndef F3Dexception_h
#define F3Dexception_h

#include <stdexcept>
#include <string>

class F3DExNoProcess : public std::runtime_error
{
public:
  explicit F3DExNoProcess(const std::string& what = "")
    : std::runtime_error(what)
  {
  }
};

class F3DExFailure : public std::runtime_error
{
public:
  explicit F3DExFailure(const std::string& what = "")
    : std::runtime_error(what)
  {
  }
};

#endif
