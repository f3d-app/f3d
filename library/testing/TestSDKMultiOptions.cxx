#include <engine.h>
#include <interactor.h>
#include <log.h>
#include <scene.h>
#include <window.h>

#include "TestSDKHelpers.h"

#include <iostream>

int TestSDKMultiOptions(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::options& opt = eng.getOptions();
  f3d::window& win = eng.getWindow();

  std::string rightFilename = "mb/mb_1_0.vtp";
  std::string leftFilename = "mb/mb_2_0.vtp";
  std::string left = std::string(argv[1]) + "data/" + leftFilename;
  std::string right = std::string(argv[1]) + "data/" + rightFilename;

  // Render one geometry with a render option
  sce.add(left);
  opt.render.show_edges = true;
  opt.render.grid.enable = true;
  opt.ui.metadata = true;
  opt.model.material.roughness = 0.6;
  win.render();

  // Add another geometry
  sce.add(right);

  // Check rendering is correct
  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKMultiOptions")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
