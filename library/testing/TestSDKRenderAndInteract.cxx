#include <engine.h>
#include <interactor.h>
#include <scene.h>
#include <window.h>

int TestSDKRenderAndInteract(int argc, char* argv[])
{
  // Order of allocation matter for VTK
  // This tests ensure that rendering then using
  // an interactor works
  // Using an onscreen window to mimic standard usage

  f3d::engine eng = f3d::engine::create();
  f3d::scene& sce = eng.getScene();
  sce.add(std::string(argv[1]) + "/data/cow.vtp");
  f3d::window& win = eng.getWindow();

  win.render();

  f3d::interactor& inter = eng.getInteractor();
  inter.start(1, [&inter]() { inter.stop(); });

  return EXIT_SUCCESS;
}
