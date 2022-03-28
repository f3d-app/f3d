#include <f3d_engine.h>
#include <f3d_interactor.h>
#include <f3d_loader.h>
#include <f3d_window.h>

int simple_sdk_test(int argc, char* argv[])
{
  f3d::engine eng(f3d::engine::WindowTypeEnum::WINDOW_STANDARD);
  f3d::loader& load = eng.getLoader();
  load.addFile(std::string(argv[1]) + "/data/cow.vtp");
  load.loadFile(f3d::loader::LoadFileEnum::LOAD_CURRENT);
  f3d::window& win = eng.getWindow();

  //  win.render(); // not ok to call before the interactor because of
  //  https://gitlab.kitware.com/vtk/vtk/-/issues/18372. Alternatively, we can force the creation of
  //  a interactor, but this should be fixed in VTK.

  // win.renderToFile(std::string(argv[2]) + "simple_sdk_test.png");

  f3d::interactor& inter = eng.getInteractor();
  //  inter.createTimerCallBack(2.0, [&inter]() { inter.stop(); });
  inter.start();
  //  inter.playInteraction(std::string(argv[1]) + "recordings/TestInteractionMisc.log");

  return 0;
}
