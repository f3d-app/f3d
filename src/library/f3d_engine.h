/**
 * @class   engine
 * @brief   Class used to create instance of other classes
 *
 * TODO improve doc
 */

#ifndef f3d_engine_h
#define f3d_engine_h

#include <bitset>
#include <memory>

namespace f3d
{
class options;
class window;
class loader;
class interactor;
class engine
{
public:

  //======== Engine Flags =============
  // engine::NO_WINDOW: Do not create a window to render into
  // engine::NO_INTERACTOR: Do not create an interactor to interact with
  // engine::WINDOW_OFFSCREEN: Create an offscreen window to render into, ignored with NO_WINDOW
  using engine_flags_t = std::bitset<8>;
  static constexpr engine_flags_t FLAGS_NONE = engine_flags_t(0x0000);
  static constexpr engine_flags_t NO_WINDOW = engine_flags_t(0x0001);
  static constexpr engine_flags_t NO_INTERACTOR = engine_flags_t(0x0002);
  static constexpr engine_flags_t WINDOW_OFFSCREEN = engine_flags_t(0x0004);

  engine(const engine_flags_t& flags = engine_flags_t());
  ~engine();

  options& getOptions();
  window& getWindow();
  loader& getLoader();
  interactor& getInteractor();

  static void printVersion();
  static void printReadersList();

private:
  class F3DInternals;
  std::unique_ptr<F3DInternals> Internals;
};
}

#endif
