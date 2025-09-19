#include <engine.h>
#include <interactor.h>
#include <window.h>

#include "TestSDKHelpers.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

int TestSDKDropZoneDeprecated([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();
  win.setSize(300, 300);

  // --- Test using the DEPRECATED option ---

  opt.ui.drop_zone.enable = true;
  opt.ui.drop_zone.show_logo = true;
  opt.ui.drop_zone.info = "Drop a file to open it\nPress H to show cheatsheet";
  win.render();

  bool deprecatedOptionResult1 = TestSDKHelpers::RenderTest(eng.getWindow(),
    std::string(argv[1]) + "baselines/", std::string(argv[2]), "TestSDKDropZoneDeprecated");

  if (!deprecatedOptionResult1)
  {
    std::cerr << "[ERROR] DEPRECATED drop_zone.info option render test failed" << '\n';
  }

  opt.ui.dropzone = true;
  opt.ui.dropzone_info = "Drop a file to open it\nPress H to show cheatsheet";
  win.render();

  bool deprecatedOptionResult2 = TestSDKHelpers::RenderTest(eng.getWindow(),
    std::string(argv[1]) + "baselines/", std::string(argv[2]), "TestSDKDropZoneDeprecated");

  if (!deprecatedOptionResult2)
  {
    std::cerr << "[ERROR] DEPRECATED dropzone_info option render test failed" << '\n';
  }

  return (deprecatedOptionResult1 && deprecatedOptionResult2) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
