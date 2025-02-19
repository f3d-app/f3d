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

  // Test clear
#if F3D_MODULE_UI
  inter.triggerCommand("print_scene_info");
  test("triggerCommand clear", (inter.triggerCommand("clear") == true));
#endif

  // Test toggle
  inter.triggerCommand("toggle model.scivis.cells");
  test("triggerCommand toggle", options.model.scivis.cells == true);

  // Test alias command
  inter.triggerCommand("alias axis_on set ui.axis on");
  inter.triggerCommand("axis_on");
  test("triggerCommand alias resolve", options.ui.axis == false);
  bool alias_result;
  alias_result = inter.triggerCommand("alias axis_off");
  test("triggerCommand alias invalid args", alias_result == false);

  // triggerCommand error codepaths
  test("triggerCommand toggle incompatible",
    inter.triggerCommand("toggle scene.animation.index") == false);
  test("triggerCommand reset inexistent", inter.triggerCommand("reset inexistent") == false);
  test("triggerCommand print not set", inter.triggerCommand("print render.hdri.file") == false);
  test("triggerCommand set unparsable",
    inter.triggerCommand("set scene.animation.index invalid") == false);

  // Add/Remove command
  inter.addCommand(
    "test_toggle", [&](const std::vector<std::string>&) { options.toggle("model.scivis.cells"); });
  inter.triggerCommand("test_toggle");
  test("addCommand", options.model.scivis.cells == false);

  inter.removeCommand("test_toggle");
  test("removeCommand", inter.triggerCommand("test_toggle") == false);

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

  // remove all commands
  for (const std::string& action : inter.getCommandActions())
  {
    inter.removeCommand(action);
  }
  test("removeAllCommands", inter.triggerCommand("print model.scivis.cells") == false);

  // Initialize default two times and check they work
  inter.initCommands();
  inter.initCommands();
  inter.triggerCommand("toggle model.scivis.cells");
  test("triggerCommand after defaults creation", options.model.scivis.cells == true);

  // check exception
  test.expect<f3d::interactor::already_exists_exception>("add already existing command", [&]() {
    inter.addCommand(
      "toggle", [&](const std::vector<std::string>&) { options.toggle("model.scivis.cells"); });
  });

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

  // check runtime exception
  test.expect<f3d::interactor::command_runtime_exception>("trigger a runtime exception", [&]() {
    inter.addCommand("exception", [&](const std::vector<std::string>&) {
      throw std::runtime_error("testing runtime exception");
    });
    inter.triggerCommand("exception");
  });

  return test.result();
}
