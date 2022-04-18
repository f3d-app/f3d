#include <f3d_engine.h>
#include <f3d_interactor.h>
#include <f3d_loader.h>
#include <f3d_window.h>

int TestSDKRenderAndInteract(int argc, char* argv[])
{
  // Order of allocation matter for VTK
  // This tests ensure that rendering then creating
  // an interactor works

  f3d::engine eng;
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/cow.vtp");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  f3d::window& win = eng.getWindow();

  win.render();

  f3d::interactor& inter = eng.getInteractor();
  inter.createTimerCallBack(1000, [&inter]() { inter.stop(); });
  inter.start();

  return EXIT_SUCCESS;
}
