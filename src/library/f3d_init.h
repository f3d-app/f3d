/**
 * @class   init
 * @brief   Class used to initialize libf3d
 *
 * TODO improve doc
 */

#ifndef f3d_init_h
#define f3d_init_h

#include <memory>

namespace f3d
{
class init
{
public:
    init();
    ~init() = default;

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif
