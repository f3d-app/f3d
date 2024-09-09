#include <export.h>
#include <options.h>

#include "FakeUnitTest.h"

#include <iostream>

int TestSDKOptions(int argc, char* argv[])
{
  FakeUnitTest test;

  f3d::options opt;

  // Test bool
  opt.setAsString("model.scivis.cells", "false");
  test("setAsString bool", opt.getAsString("model.scivis.cells") == "false");

  opt.setAsString("model.scivis.cells", "1");
  test("setAsString bool", opt.getAsString("model.scivis.cells") == "true");

  opt.model.scivis.cells = false;
  test("getAsString bool", opt.getAsString("model.scivis.cells") == "false");

  opt.toggle("model.scivis.cells");
  test("toggle", opt.getAsString("model.scivis.cells") == "true");

  // Test int
  opt.setAsString("scene.animation.index", "2");
  test("setAsString int", opt.getAsString("scene.animation.index") == "2");

  opt.scene.animation.index = 3;
  test("getAsString int", opt.getAsString("scene.animation.index") == "3");

  // Test double
  opt.setAsString("render.line_width", "2.14");
  test("setAsString double", opt.getAsString("render.line_width") == "2.14");

  opt.render.line_width = 2.13;
  test("getAsString double", opt.getAsString("render.line_width") == "2.13");

  // Test ratio_t
  opt.setAsString("scene.animation.speed_factor", "3.17");
  test("setAsString ratio_t", opt.getAsString("scene.animation.speed_factor") == "3.17");

  opt.scene.animation.speed_factor = 3.18;
  test("getAsString ratio_t", opt.getAsString("scene.animation.speed_factor") == "3.18");

  // Test string
  opt.setAsString("model.color.texture", "testAsString");
  test("setAsString string", opt.getAsString("model.color.texture") == "testAsString");

  opt.model.color.texture = "testInStruct";
  test("getAsString string", opt.getAsString("model.color.texture") == "testInStruct");

  // Test double vector
  opt.setAsString("render.background.color", "0.1, 0.2, 0.4");
  test("setAsString vector<double>", opt.getAsString("render.background.color") == "0.1, 0.2, 0.4");

  opt.render.background.color = { 0.1, 0.2, 0.5 };
  test("getAsString vector<double>", opt.getAsString("render.background.color") == "0.1, 0.2, 0.5");

  // Test closest option
  auto closest = opt.getClosestOption("modle.sciivs.cell");
  test("closest option", closest.first == "model.scivis.cells" && closest.second == 5);

  closest = opt.getClosestOption("model.scivis.cells");
  test("closest option exact", closest.first == "model.scivis.cells" && closest.second == 0);

  // Test chaining options
  opt.setAsString("model.scivis.cells", "false").setAsString("model.scivis.cells", "true");
  test("chaining setAsString calls", opt.getAsString("model.scivis.cells") == "true");

  // Test toggle error paths
  test.expect<f3d::options::incompatible_exception>(
    "toggle non-bool", [&]() { opt.toggle("render.line_width"); });

  // Test parsing error path
  test.expect<f3d::options::parsing_exception>(
    "set invalid int string", [&]() { opt.setAsString("scene.animation.index", "invalid"); });

  test.expect<f3d::options::parsing_exception>("set out-of-range int string",
    [&]() { opt.setAsString("scene.animation.index", "2147483648"); });

  test.expect<f3d::options::parsing_exception>(
    "set invalid double string", [&]() { opt.setAsString("render.line_width", "invalid"); });

  test.expect<f3d::options::parsing_exception>("set out-of-range double string", [&]() {
    opt.setAsString("render.line_width",
      "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012"
      "34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234"
      "56789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456"
      "78901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"
      "90123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
      "1234567890123456789012345678901234567890");
  });

  // Test copy operator and constructor
  f3d::options opt2 = opt;
  test("copy constructor", opt2.render.line_width == 2.13);

  f3d::options opt3;
  opt3 = opt2;
  test("copy operator", opt3.render.line_width == 2.13);

  f3d::options opt4 = std::move(opt3);
  test("move constructor", opt4.render.line_width == 2.13);

  f3d::options opt5;
  opt5 = std::move(opt4);
  test("move operator", opt5.render.line_width == 2.13);

  // Test getNames
  std::vector<std::string> names = opt.getNames();
  test("getNames count", names.size() != 0 && names == opt2.getNames());

  // Test isSame/copy
  test("isSame", opt.isSame(opt2, "render.line_width"));

  opt2.render.line_width = 3.12;
  test("not isSame", !opt.isSame(opt2, "render.line_width"));

  opt2.copy(opt, "render.line_width");
  test("copy", opt2.render.line_width == 2.13);

  // Test isSame/copy vector
  test("isSame with vectors", opt.isSame(opt2, "render.background.color"));

  opt2.render.background.color = { 0.1, 0.2, 0.6 };
  test("not isSame with vectors", !opt.isSame(opt2, "render.background.color"));

  opt2.copy(opt, "render.background.color");
  test("copy with vectors", opt2.render.background.color == std::vector<double>({ 0.1, 0.2, 0.5 }));

  // Test isSame/copy error path
  test.expect<f3d::options::inexistent_exception>(
    "inexistent_exception exception on isSame", [&]() { opt.isSame(opt2, "dummy"); });

  test.expect<f3d::options::inexistent_exception>(
    "inexistent_exception exception on copy", [&]() { opt.copy(opt2, "dummy"); });

  // Test parse
  test("parse bool", f3d::options::parse<bool>("true"), true);
  test("parse int", f3d::options::parse<int>("37"), 37);
  test("parse double", f3d::options::parse<double>("37.2"), 37.2);
  test("parse ratio", f3d::options::parse<f3d::ratio_t>("0.31"), 0.31);
  test("parse string", f3d::options::parse<std::string>("foobar"), "foobar");
  test("parse vector<double>",
    f3d::options::parse<std::vector<double>>("0.1, 0.2, 0.3") ==
      std::vector<double>({ 0.1, 0.2, 0.3 }));

  return test.result();
}
