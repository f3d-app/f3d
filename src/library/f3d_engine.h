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
  struct window_exception : public std::exception
  {
    const char* what() const throw() { return "Cannot create window with this engine"; }
  };
  struct interactor_exception : public std::exception
  {
    const char* what() const throw() { return "Cannot create interactor with this engine"; }
  };

  //======== Engine Flags =============
  // engine::CREATE_WINDOW: Create a window to render into.
  // engine::CREATE_INTERACTOR: Create an interactor to interact with
  // engine::WINDOW_OFFSCREEN: Create an offscreen window to render into, need CREATE_WINDOW
  using flags_t = int;
  enum Flags : flags_t
  {
    FLAGS_NONE = 0,             // 0000
    CREATE_WINDOW = 1 << 0,     // 0001
    CREATE_INTERACTOR = 1 << 1, // 0010
    WINDOW_OFFSCREEN = 1 << 2   // 0100
  };

  engine(const flags_t& flags);
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
