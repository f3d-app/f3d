#include <f3d_loader.h>
#include <f3d_options.h>
#include <f3d_interactor.h>

int main()
{
  f3d::options options;
  f3d::loader loader(options);
  f3d::interactor interactor;
  loader.setInteractor(&interactor);

  // TODO Actual test

  return 0;
}
