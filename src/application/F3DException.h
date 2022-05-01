#ifndef F3Dexception_h
#define F3Dexception_h

#include <exception>
#include <string>

struct F3DExNoProcess : std::exception
{
  const char* what() const throw() { return "no process required"; }
};

#endif
