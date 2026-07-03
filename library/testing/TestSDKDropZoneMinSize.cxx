#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <iostream>
#include <window.h>

int TestSDKDropZoneMinSize(int, char* argv[])
{
  PseudoUnitTest test;

  std::string renderingBackend = std::string(argv[4]);
  f3d::engine eng = TestSDKHelpers::CreateOffscreenEngine(renderingBackend);
  f3d::window& win = eng.getWindow();
  f3d::options& opt = eng.getOptions();

  win.setSize(5, 5);

  opt.ui.drop_zone.enable = true;
  opt.ui.drop_zone.show_logo = true;
  opt.ui.drop_zone.custom_binds = "None+Drop Ctrl+O None+H";

  test("Render with min size", [&]() { win.render(); });
  return test.result();
}
