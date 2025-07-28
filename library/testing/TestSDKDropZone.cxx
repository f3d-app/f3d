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

  // --- Test using the NEW option ---
  std::cout << "[DEBUG] Running DropZone test with NEW drop_zone option" << '\n';
  opt.ui.drop_zone.enable = true;
  opt.ui.drop_zone.show_logo = true;
  opt.ui.drop_zone.info = "Drop a file to open it\nPress H to show cheatsheet";
  win.render();

  bool newOptionResult = TestSDKHelpers::RenderTest(
    eng.getWindow(), std::string(argv[1]) + "baselines/", std::string(argv[2]), "TestSDKDropZone");

  // --- Test using the DEPRECATED option ---
  std::cout << "[DEBUG] Running DropZone test with DEPRECATED dropzone option" << '\n';
  opt.ui.dropzone = true;
  opt.ui.dropzone_info = "Drop a file to open it\nPress H to show cheatsheet";
  win.render();

  bool deprecatedOptionResult = TestSDKHelpers::RenderTest(
    eng.getWindow(), std::string(argv[1]) + "baselines/", std::string(argv[2]), "TestSDKDropZone");

  if (!newOptionResult)
  {
    std::cerr << "[ERROR] NEW drop_zone option render test failed" << std::endl;
  }
  if (!deprecatedOptionResult)
  {
    std::cerr << "[ERROR] DEPRECATED dropzone option render test failed" << std::endl;
  }

  return (newOptionResult && deprecatedOptionResult) ? EXIT_SUCCESS : EXIT_FAILURE;
}
