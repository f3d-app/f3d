/**
 * @class   init
 * @brief   A private class used to initialize libf3d
 *
 * A private class used to initialize libf3d, it relies on a global static instance
 * that will call the necessary code when loading the libf3d symbols
 */

#ifndef f3d_init_h
#define f3d_init_h

#include <memory>

namespace f3d::detail
{
class init
{
public:
  /**
   * libf3d initialization:
   *  - Control VTK global warning mechanism
   *  - Control VTK logger behavior
   *  - Create and register VTK factories
   *  - Set log verbose level to info to initialize the output window
   */
  init();

  /**
   * Default destructor
   */
  ~init();

private:
  class internals;
  std::unique_ptr<internals> Internals;
};
}

static const f3d::detail::init gInitInstance;
#endif
