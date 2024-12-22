#include <engine.h>
#include <interactor.h>
#include <window.h>

#include "TestSDKHelpers.h"

int TestSDKDropZone(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);
  opt.ui.dropzone = true;
  opt.ui.dropzone_info = "Drop a file to open it\nPress H to show cheatsheet";

  win.render();

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKDropZone")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
