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
  if (!inter.playInteraction(
        interactionFilePath)) // Dragon.vtu; SSS; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; 7
  {
    std::cerr << "Unexcepted error playing interaction" << std::endl;
    return EXIT_FAILURE;
  }
  if (!TestSDKHelpers::RenderTest(
        win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename + "Default"))
  {
    return EXIT_FAILURE;
  }

  // Remove some default interactions to reduce false positive risks

  // Do not remove "S", to ensure it is being replaced as expected
  inter.removeInteractionCommands("X", f3d::interactor::ModifierKeys::ANY);
  inter.removeInteractionCommands("N", f3d::interactor::ModifierKeys::ANY);
  inter.removeInteractionCommands("G", f3d::interactor::ModifierKeys::ANY);
  inter.removeInteractionCommands("E", f3d::interactor::ModifierKeys::ANY);
  inter.removeInteractionCommands("P", f3d::interactor::ModifierKeys::ANY);
  inter.removeInteractionCommands("Y", f3d::interactor::ModifierKeys::ANY);
  inter.removeInteractionCommands("B", f3d::interactor::ModifierKeys::ANY);

  // Remove an interaction that will be triggered later and should not have any effect
  inter.removeInteractionCommands("7", f3d::interactor::ModifierKeys::ANY);

  // Check that an ANY interaction can be added and that it removes existing interaction
  inter.addInteractionCommands(
    "S", f3d::interactor::ModifierKeys::ANY, { "set interactor.axis true" });

  // Check CTRL modifier and that another interaction can be added on the same key with another
  // modifier
  inter.addInteractionCommands(
    "S", f3d::interactor::ModifierKeys::CTRL, { "set render.grid.enable true" });

  // Check invalid command for coverage
  inter.addInteractionCommands("P", f3d::interactor::ModifierKeys::ANY, { "invalid command any" });
  inter.addInteractionCommands(
    "P", f3d::interactor::ModifierKeys::CTRL, { "invalid command ctrl" });

  // Check SHIFT modifier
  inter.addInteractionCommands(
    "Y", f3d::interactor::ModifierKeys::SHIFT, { R"(set ui.filename_info "My Own Filename")" });

  // Check CTRL_SHIFT modifier
  inter.addInteractionCommands("B", f3d::interactor::ModifierKeys::CTRL_SHIFT,
    { "set ui.filename true", "set render.show_edges true" });

  // Check drop files callback
  inter.setDropFilesCallBack([&](std::vector<std::string> filesVec) -> bool {
    std::string path = filesVec[0];
    size_t found = path.find_last_of("/\\");
    sce.clear();
    sce.add(path.substr(0, found + 1) + "suzanne.ply");
    win.render();
    return true;
  });

  // This time the interaction should result in a different rendering
  if (!inter.playInteraction(
        interactionFilePath)) // Dragon.vtu; SSS; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; 7
  {
    std::cerr << "Unexcepted error playing interaction" << std::endl;
    return EXIT_FAILURE;
  }

  return TestSDKHelpers::RenderTest(
           win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename + "Modified")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
