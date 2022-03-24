#include <f3d_engine.h>
#include <f3d_interactor.h>
#include <f3d_log.h>
#include <f3d_options.h>
#include <f3d_window.h>

int main()
{
  f3d::engine::WindowTypeEnum type = f3d::engine::WindowTypeEnum::WINDOW_STANDARD;
  f3d::engine(type);

  // TODO Actual test

  return 0;
}
