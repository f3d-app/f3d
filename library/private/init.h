/**
 * @class   init
 * @brief   A private class used to initialize libf3d
 *
 * A private class used to initialize libf3d, it relies on a static initialization
 * method that creates an static internal instance.
 * that will call the necessary code when loading the libf3d symbols
 */

#ifndef f3d_init_h
#define f3d_init_h

namespace f3d::detail
{
class init
{
public:
  /**
   * Internally create an init static instance
   */
  static void initialize();

  /**
   * libf3d initialization:
   *  - Control VTK global warning mechanism
   *  - Control VTK logger behavior
   *  - Create and register VTK factories
   *  - Set log verbose level to info to initialize the output window
   *  - Register additional image readers
   *
   * Public is needed because initialize uses make_shared
   */
  init();

  /**
   * Default destructor
   */
  ~init() = default;
};
}
#endif
