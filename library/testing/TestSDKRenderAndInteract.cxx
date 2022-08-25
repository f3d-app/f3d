#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <window.h>

int TestSDKRenderAndInteract(int argc, char* argv[])
{
  // Order of allocation matter for VTK
  // This tests ensure that rendering then using
  // an interactor works
  // Using an onscreen window to mimic standard usage

  f3d::engine eng(f3d::window::Type::NATIVE);
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/cow.vtp").loadFile();
  f3d::window& win = eng.getWindow();

  win.render();

  f3d::interactor& inter = eng.getInteractor();
  inter.createTimerCallBack(1000, [&inter]() { inter.stop(); });
  inter.start();

  return EXIT_SUCCESS;
}
