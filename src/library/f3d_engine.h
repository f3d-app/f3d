/**
 * @class   engine
 * @brief   Class used to create instance of other classes
 *
 * TODO improve doc
 */

#ifndef f3d_engine_h
#define f3d_engine_h

#include "f3d_export.h"

#include <string>

namespace f3d
{
class options;
class window;
class loader;
class interactor;
class F3D_EXPORT engine
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
  // engine::CREATE_INTERACTOR: Create an interactor to interact with.
  // engine::WINDOW_OFFSCREEN: Create an offscreen window to render into, need CREATE_WINDOW.
  using flags_t = uint32_t;
  enum Flags : flags_t
  {
    FLAGS_NONE = 0,             // 0000
    CREATE_WINDOW = 1 << 0,     // 0001
    CREATE_INTERACTOR = 1 << 1, // 0010
    WINDOW_OFFSCREEN = 1 << 2   // 0100
  };

  engine(const flags_t& flags);
  ~engine();

  // Engine provide a default options
  // use this setter to use other options
  void setOptions(f3d::options& options);

  options& getOptions();
  window& getWindow();
  loader& getLoader();
  interactor& getInteractor();

  // TODO: remove this function
  static const std::string& getAppTitle();

  static void printVersion();
  static void printReadersList();

private:
  class F3DInternals;
  F3DInternals* Internals;
  engine(const engine& opt) = delete;
  engine& operator=(const engine& opt) = delete;
};
}

#endif
