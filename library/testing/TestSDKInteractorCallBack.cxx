#include <engine.h>
#include <interactor.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include <iostream>

#include "TestSDKHelpers.h"

int TestSDKInteractorCallBack(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::window& win = eng.getWindow();
  f3d::interactor& inter = eng.getInteractor();
  win.setSize(300, 300);

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

  // Dragon.vtu; SYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
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

  // Remove interactions that will be triggered later and should not have any effect
  // Do not remove S as it will be replaced below
  inter.removeInteractionCommands("7", f3d::interactor::ModifierKeys::ANY);
  inter.removeInteractionCommands("Y", f3d::interactor::ModifierKeys::NONE);
  inter.removeInteractionCommands("B", f3d::interactor::ModifierKeys::NONE);

  // Check that an interaction can be added and that it removes existing interaction
  inter.replaceInteractionCommand(
    "S", f3d::interactor::ModifierKeys::NONE, "toggle interactor.axis");

  // Check CTRL modifier and that another interaction can be added on the same key with another
  // modifier
  inter.addInteractionCommand(
    "S", f3d::interactor::ModifierKeys::CTRL, "toggle render.grid.enable");

  // Check invalid command for coverage
  inter.addInteractionCommand("P", f3d::interactor::ModifierKeys::CTRL, "invalid command");

  // Check SHIFT modifier
  inter.addInteractionCommand(
    "Y", f3d::interactor::ModifierKeys::SHIFT, R"(set ui.filename_info "My Own Filename")");

  // Check CTRL_SHIFT modifier
  inter.addInteractionCommands("B", f3d::interactor::ModifierKeys::CTRL_SHIFT,
    { "set ui.filename true", "set render.show_edges true" });

  // Check ANY modifier
  inter.addInteractionCommand(
    "A", f3d::interactor::ModifierKeys::ANY, "toggle render.background.skybox");

  // Modify the add_files command
  inter.replaceCommandCallback("add_files", [&](const std::vector<std::string>& filesVec) -> bool {
    const std::string& path = filesVec[0];
    size_t found = path.find_last_of("/\\");
    sce.clear();
    sce.add(path.substr(0, found + 1) + "suzanne.ply");
    return true;
  });

  // This time the interaction should result in a different rendering
  // Dragon.vtu; SYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
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

  // Remove all interactions and play interaction again
  inter.removeAllInteractionsCommands();
  // Dragon.vtu; SYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
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

  // Create default interactions commands again, two times, and check rendering
  inter.createDefaultInteractionsCommands();
  inter.createDefaultInteractionsCommands();
  // Dragon.vtu; SYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
  if (!inter.playInteraction(interactionFilePath))
  {
    std::cerr << "Unexcepted error playing interaction" << std::endl;
    return EXIT_FAILURE;
  }
  return TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", std::string(argv[2]),
           filename + "DefaultAgain")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
