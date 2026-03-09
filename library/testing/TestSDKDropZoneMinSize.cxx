#include <engine.h>
#include <iostream>
#include <window.h>

int TestSDKDropZoneMinSize(int, char*[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();

  win.setSize(5, 5);

  opt.ui.drop_zone.enable = true;
  opt.ui.drop_zone.show_logo = true;
  opt.ui.drop_zone.custom_binds = "None+Drop Ctrl+O None+H";

  try
  {
    win.render();
  }
  catch (const std::exception& e)
  {
    std::cerr << "Unexpected error during render: " << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
