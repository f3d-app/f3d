/**
 * @class   engine
 * @brief   Class used to create instance of other classes
 *
 * TODO improve doc
 */

#ifndef f3d_engine_h
#define f3d_engine_h

#include "f3d_init.h"

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
  enum class WindowTypeEnum
  {
    WINDOW_NO_RENDER,
    WINDOW_STANDARD
  };

  engine(WindowTypeEnum windowType, bool offscreen);
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

  // TODO use binary flags instead
  WindowTypeEnum WindowType;
  bool Offscreen = false;
};
}

#endif
