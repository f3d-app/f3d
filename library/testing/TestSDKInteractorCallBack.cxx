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

  constexpr int expectedBindingsSize = 39;

  // Check the current number of documented bindings
  if (inter.getBindingsDocumentation().size() != expectedBindingsSize)
  {
    std::cerr << "Unexcepted bindings documentation size initially" << std::endl;
    return EXIT_FAILURE;
  }

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
    inter.addBinding("7", f3d::interactor::ModifierKeys::ANY, "exception");
    std::cerr << "An exception has not been thrown when adding a existing interaction command"
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::interactor::already_exists_exception& ex)
  {
  }

  // Remove bindings that will be triggered later and should not have any effect
  inter.removeBinding("7", f3d::interactor::ModifierKeys::ANY);
  inter.removeBinding("Y", f3d::interactor::ModifierKeys::NONE);
  inter.removeBinding("B", f3d::interactor::ModifierKeys::NONE);
  inter.removeBinding("S", f3d::interactor::ModifierKeys::NONE);
  inter.removeBinding("Z", f3d::interactor::ModifierKeys::NONE);

  // Check the current number of documented bindings
  if (inter.getBindingsDocumentation().size() != expectedBindingsSize - 5)
  {
    std::cerr << "Unexcepted bindings documentation size after removal" << std::endl;
    return EXIT_FAILURE;
  }

  // Check that an binding can be added
  inter.addBinding("S", f3d::interactor::ModifierKeys::NONE, "toggle interactor.axis");

  // Check CTRL modifier and that another interaction can be added on the same key with another
  // modifier
  inter.addBinding("S", f3d::interactor::ModifierKeys::CTRL, "toggle render.grid.enable");

  // Check invalid command for coverage
  inter.addBinding("P", f3d::interactor::ModifierKeys::CTRL, "invalid command");

  // Check SHIFT modifier
  inter.addBinding(
    "Y", f3d::interactor::ModifierKeys::SHIFT, R"(set ui.filename_info "My Own Filename")");

  // Check CTRL_SHIFT modifier
  inter.addBinding("B", f3d::interactor::ModifierKeys::CTRL_SHIFT,
    { "set ui.filename true", "set render.show_edges true" });

  // Check ANY modifier
  inter.addBinding("A", f3d::interactor::ModifierKeys::ANY, "toggle render.background.skybox");

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
  inter.addBinding("Z", f3d::interactor::ModifierKeys::NONE, "exception");

  // Check the current number of documented bindings
  if (inter.getBindingsDocumentation().size() != expectedBindingsSize - 5)
  {
    std::cerr << "Unexcepted bindings documentation size after addition without doc" << std::endl;
    return EXIT_FAILURE;
  }

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
  inter.removeBinding("Invalid", f3d::interactor::ModifierKeys::ANY);

  // Remove all bindings
  for (const auto& [interaction, modifier] : inter.getBindingInteractions())
  {
    inter.removeBinding(interaction, modifier);
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

  // Check the current number of documented bindings
  if (inter.getBindingsDocumentation().size() != 0)
  {
    std::cerr << "Unexcepted bindings documentation size after complete removal" << std::endl;
    return EXIT_FAILURE;
  }

  // initialize default bindings again, two times, and check rendering
  inter.initBindings();
  inter.initBindings();

  // Check the current number of documented bindings
  if (inter.getBindingsDocumentation().size() != expectedBindingsSize)
  {
    std::cerr << "Unexcepted bindings documentation size after initialization" << std::endl;
    return EXIT_FAILURE;
  }

  // Dragon.vtu; SZZYB; CTRL+S; CTRL+P; SHIFT+Y; CTRL+SHIFT+B; CTRL+SHIFT+A; 7
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
