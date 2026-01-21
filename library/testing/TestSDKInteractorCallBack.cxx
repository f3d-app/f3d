#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <interactor.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include <iostream>

using mod_t = f3d::interaction_bind_t::ModifierKeys;

int TestSDKInteractorCallBack([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::interactor& inter = eng.getInteractor();
  win.setSize(300, 300);
  win.render();

  // Sanity checks coverage
  test("play empty interaction", !inter.playInteraction(""));
  test("record empty interaction", !inter.recordInteraction(""));

  // Test callbacks with default interactions
  std::string filename = "TestSDKInteractorCallBack";
  std::string interactionFilePath = std::string(argv[2]) + "../../" + filename + ".log";

  // Dragon.vtu; SZZYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
  test("play some interactions", inter.playInteraction(interactionFilePath));
  test("render interaction result",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename + "Default"));

  // Check that adding an existing interaction command trigger an exception

  test.expect<f3d::interactor::already_exists_exception>(
    "add existing interaction", [&]() { inter.addBinding({ mod_t::ANY, "7" }, "exception"); });

  // Remove bindings that will be triggered later and should not have any effect
  inter.removeBinding({ mod_t::ANY, "7" });
  inter.removeBinding({ mod_t::NONE, "Y" });
  inter.removeBinding({ mod_t::NONE, "B" });
  inter.removeBinding({ mod_t::NONE, "S" });
  inter.removeBinding({ mod_t::NONE, "Z" });
  inter.removeBinding({ mod_t::CTRL, "P" });

  // Check that an binding can be added
  inter.addBinding({ mod_t::NONE, "S" }, "toggle ui.axis");

  // Check CTRL modifier and that another interaction can be added on the same key with another
  // modifier
  inter.addBinding({ mod_t::CTRL, "S" }, "toggle render.grid.enable");

  // Check invalid command for coverage
  inter.addBinding({ mod_t::CTRL, "P" }, "invalid command");

  // Check SHIFT modifier
  inter.addBinding({ mod_t::SHIFT, "Y" }, R"(set ui.filename_info "My Own Filename")");

  // Check CTRL_SHIFT modifier
  inter.addBinding(
    { mod_t::CTRL_SHIFT, "B" }, { "set ui.filename true", "set render.show_edges true" });

  // Check ANY modifier
  inter.addBinding({ mod_t::ANY, "A" }, "toggle render.background.skybox");

  // Replace the add_files command
  inter.removeCommand("add_files");
  inter.addCommand("add_files", [&](const std::vector<std::string>& filesVec) {
    const std::string& path = filesVec[0];
    size_t found = path.find_last_of("/\\");
    sce.clear();
    sce.add(path.substr(0, found + 1) + "suzanne.ply");
  });

  // Add a command and binding that throws an exception
  inter.addCommand("exception", [&](const std::vector<std::string>&) {
    throw std::runtime_error("testing runtime exception");
  });
  inter.addBinding({ mod_t::NONE, "Z" }, "exception");

  // This time the interaction should result in a different rendering
  // Dragon.vtu; SZZYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
  test("play interactions after modifications", inter.playInteraction(interactionFilePath));

  // With VTK 9.3.0, rendering is slightly different
  test("render modified interaction result",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename + "Modified", 0.11));

  // Remove a non-existing interaction command
  inter.removeBinding({ mod_t::ANY, "Invalid" });

  // Remove all bindings
  for (const std::string& group : inter.getBindGroups())
  {
    for (const f3d::interaction_bind_t& bind : inter.getBindsForGroup(group))
    {
      inter.removeBinding(bind);
    }
  }

  // Play interaction again, which should not have any effect
  // Dragon.vtu; SZZYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
  test(
    "play interaction after removing all interactions", inter.playInteraction(interactionFilePath));

  // With VTK 9.3.0, rendering is slightly different
  test("render after interaction that should have had no effect",
    TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
      filename + "ModifiedAgain", 0.11));

  // initialize default bindings again, two times, and check rendering
  inter.initBindings();
  inter.initBindings();

  // Dragon.vtu; SZZYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
  test("play interactions after initialization", inter.playInteraction(interactionFilePath));
  test("render after playing defaulted interactions",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename + "DefaultAgain"));

  // Check error handling
  test("record to an invalid path",
    !inter.recordInteraction("/" + std::string(257, 'x') + "/record.ext"));
  test("play interaction from an invalid path",
    !inter.playInteraction("/" + std::string(257, 'x') + "/play.ext"));

  // Check console error handling
  // Esc;"exception";Enter;Esc
  test("play an interaction that write a command that trigger an exception internally",
    inter.playInteraction(
      std::string(argv[1]) + "recordings/TestSDKInteractorCallBackConsoleException.log"));

  return test.result();
}
