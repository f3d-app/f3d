#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>
#include <options.h>

int TestSDKInteractorCommand(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::options& options = eng.getOptions();
  f3d::interactor& inter = eng.getInteractor();

  PseudoUnitTest test;

  // Test `set`
  inter.triggerCommand("set model.scivis.cells true");
  test("triggerCommand set", options.model.scivis.cells == true);
  inter.triggerCommand("set render.hdri.file filepath");
  test("triggerCommand set double quotes", options.render.hdri.file.value() == "filepath");

  // Test reset
  inter.triggerCommand("reset model.scivis.cells");
  test("triggerCommand reset", options.model.scivis.cells == false);
  inter.triggerCommand("reset render.hdri.file");
  test("triggerCommand reset optional", options.render.hdri.file.has_value() == false);

  // Test toggle
  inter.triggerCommand("toggle model.scivis.cells");
  test("triggerCommand toggle", options.model.scivis.cells == true);

  // triggerCommand error codepaths
  test("triggerCommand toggle incompatible",
    inter.triggerCommand("toggle scene.animation.index") == false);
  test("triggerCommand reset inexistent", inter.triggerCommand("reset inexistent") == false);
  test("triggerCommand print not set", inter.triggerCommand("print render.hdri.file") == false);
  test("triggerCommand set unparsable",
    inter.triggerCommand("set scene.animation.index invalid") == false);

  // Add/Remove callback
  inter.addCommandCallback("test_toggle", [&](const std::vector<std::string>&) -> bool {
    options.toggle("model.scivis.cells");
    return true;
  });
  inter.triggerCommand("test_toggle");
  test("addCommandCallback", options.model.scivis.cells == false);

  inter.removeCommandCallback("test_toggle");
  test("removeCommandCallback", inter.triggerCommand("test_toggle") == false);

  // Coverage print
  inter.triggerCommand("print model.scivis.cells");

  // Coverage cycle_coloring
  test("triggerCommand cycle_coloring invalid arg",
    inter.triggerCommand("cycle_coloring invalid") == false);

  // Coverage set_camera
  test(
    "triggerCommand set_camera invalid arg", inter.triggerCommand("set_camera invalid") == false);

  // Coverage exception handling
  test("triggerCommand exception handling",
    inter.triggerCommand(R"(print "render.hdri.file)") == false);

  // Args check
  test("triggerCommand set invalid args", inter.triggerCommand("set one") == false);
  test("triggerCommand toggle invalid args", inter.triggerCommand("toggle one two") == false);
  test("triggerCommand unset invalid args", inter.triggerCommand("unset one two") == false);
  test("triggerCommand reset invalid args", inter.triggerCommand("reset one two") == false);
  test("triggerCommand print invalid args", inter.triggerCommand("print one two") == false);
  test("triggerCommand roll_camera invalid args",
    inter.triggerCommand("roll_camera one two") == false);
  test(
    "triggerCommand set_camera invalid args", inter.triggerCommand("set_camera one two") == false);
  test("triggerCommand cycle_coloring invalid args",
    inter.triggerCommand("cycle_coloring one two") == false);

  return test.result();
}
