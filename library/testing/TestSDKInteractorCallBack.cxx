#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <options.h>
#include <window.h>

#include <iostream>

#include "TestSDKHelpers.h"

int TestSDKInteractorCallBack(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::Type::NATIVE_OFFSCREEN);
  f3d::options& options = eng.getOptions();
  f3d::loader& load = eng.getLoader();
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

  // Test callbacks
  std::string filename = "TestSDKInteractorCallBack";
  std::string interactionFilePath = std::filesystem::path(argv[2])
                                      .parent_path()
                                      .parent_path()
                                      .parent_path()
                                      .append(filename + ".log")
                                      .string();
  inter.playInteraction(interactionFilePath); // Dragon.vtu; S

  inter.setKeyPressCallBack(
    [&](int, std::string keySym) -> bool
    {
      if (keySym == "S")
      {
        options.set("interactor.axis", true);
        win.render();
        return true;
      }
      return false;
    });

  inter.setDropFilesCallBack(
    [&](std::vector<std::string> filesVec) -> bool
    {
      std::string path = filesVec[0];
      load.addFile(std::filesystem::path(path).replace_filename("suzanne.ply").string())
        .loadFile(f3d::loader::LoadFileEnum::LOAD_LAST);
      win.render();
      return true;
    });

  // This time the interaction should result in a different rendering
  inter.playInteraction(interactionFilePath);

  return TestSDKHelpers::RenderTest(
           win, std::string(argv[1]) + "baselines/", std::string(argv[2]), filename, 50)
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
