#include <engine.h>
#include <interactor.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include <iostream>

#include "TestSDKHelpers.h"

using mod_t = f3d::interaction_bind_t::ModifierKeys;

int TestSDKInteractorCallBack(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::interactor& inter = eng.getInteractor();
  win.setSize(300, 300);
  win.render();

  // Sanity checks coverage
  if (inter.playInteraction(""))
  {
    std::cerr << "Unexcepted playInteraction output" << std::endl;
    return EXIT_FAILURE;
  }
  if (inter.recordInteraction(""))
  {
    std::cerr << "Unexcepted recordInteraction output" << std::endl;
    return EXIT_FAILURE;
  }

  // Test callbacks with default interactions
  std::string filename = "TestSDKInteractorCallBack";
  std::string interactionFilePath = std::string(argv[2]) + "../../" + filename + ".log";

  // Dragon.vtu; SZZYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
  if (!inter.playInteraction(interactionFilePath))
  {
    std::cerr << "Unexcepted error playing interaction" << std::endl;
    return EXIT_FAILURE;
  }
  if (!TestSDKHelpers::RenderTest(
        win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename + "Default"))
  {
    return EXIT_FAILURE;
  }

  // Check that adding an existing interaction command trigger an exception
  try
  {
    inter.addBinding({ mod_t::ANY, "7" }, "exception");
    std::cerr << "An exception has not been thrown when adding a existing interaction command"
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::interactor::already_exists_exception& ex)
  {
  }

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
  constexpr InteractiveCommandDoc<0> F3D_CMD_ADD_FILES = { "add_files",
    "Add files." };
  inter.addCommand(makeCommand(F3D_CMD_ADD_FILES, [&](const std::vector<std::string>& filesVec) {
    const std::string& path = filesVec[0];
    size_t found = path.find_last_of("/\\");
    sce.clear();
    sce.add(path.substr(0, found + 1) + "suzanne.ply");
  }));

  // Add a command and binding that throws an exception
  constexpr InteractiveCommandDoc<0> F3D_CMD_EXCEPTION = { "exception",
    "Testing runtime exception." };
  inter.addCommand(makeCommand(F3D_CMD_EXCEPTION, [&](const std::vector<std::string>&) {
    throw std::runtime_error("testing runtime exception");
  }));
  inter.addBinding({ mod_t::NONE, "Z" }, "exception");

  // This time the interaction should result in a different rendering
  // Dragon.vtu; SZZYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
  if (!inter.playInteraction(interactionFilePath))
  {
    std::cerr << "Unexcepted error playing interaction" << std::endl;
    return EXIT_FAILURE;
  }

  // With VTK 9.2.6 and 9.3.0, rendering is slightly different
  if (!TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
        filename + "Modified", 0.11))
  {
    return EXIT_FAILURE;
  }

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
  if (!inter.playInteraction(interactionFilePath))
  {
    std::cerr << "Unexcepted error playing interaction" << std::endl;
    return EXIT_FAILURE;
  }

  // With VTK 9.2.6 and 9.3.0, rendering is slightly different
  if (!TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
        filename + "ModifiedAgain", 0.11))
  {
    return EXIT_FAILURE;
  }

  // initialize default bindings again, two times, and check rendering
  inter.initBindings();
  inter.initBindings();

  // Dragon.vtu; SZZYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
  if (!inter.playInteraction(interactionFilePath))
  {
    std::cerr << "Unexcepted error playing interaction" << std::endl;
    return EXIT_FAILURE;
  }
  if (!TestSDKHelpers::RenderTest(
        win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename + "DefaultAgain"))
  {
    std::cerr << "Unexcepted rendering playing interaction" << std::endl;
    return EXIT_FAILURE;
  }

  // Check error handling
  if (inter.recordInteraction("/" + std::string(257, 'x') + "/record.ext"))
  {
    std::cerr << "Unexcepted success recording an invalid path" << std::endl;
    return EXIT_FAILURE;
  }
  if (inter.playInteraction("/" + std::string(257, 'x') + "/play.ext"))
  {
    std::cerr << "Unexcepted success playing an invalid path" << std::endl;
    return EXIT_FAILURE;
  }

  // Check console error handling
  // Esc;"exception";Enter;Esc
  if (!inter.playInteraction(
        std::string(argv[1]) + "recordings/TestSDKInteractorCallBackConsoleException.log"))
  {
    std::cerr << "Unexcepted failure playing a console command triggering an exception"
              << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
