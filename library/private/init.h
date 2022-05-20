/**
 * @class   init
 * @brief   Class used to initialize libf3d
 *
 * TODO improve doc
 */

#ifndef f3d_init_h
#define f3d_init_h

#include <memory>

namespace f3d::detail
{
class init
{
public:
  init();
  ~init();

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}

static const f3d::detail::init gInitInstance;
#endif
