#ifndef f3d_exception_h
#define f3d_exception_h

#include <stdexcept>
#include <string>

namespace f3d
{
/**
 * @class   exception
 * @brief   a generic exception used in the libf3d
 *
 * A generic exception used in the libf3d.
 * Can be used to catch all exceptions when needed.
 */
struct exception : public std::runtime_error
{
  exception(const std::string& what = "")
    : std::runtime_error(what)
  {
  }
};
}

#endif
