#include <engine.h>
#include <interactor.h>
#include <log.h>
#include <scene.h>
#include <window.h>

#include "TestSDKHelpers.h"

#include <iostream>

int TestSDKMultiColoring(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::ERROR);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::options& opt = eng.getOptions();

  // Test file logic
  std::string cubeFilename = "mb/mb_0_0.vtu";
  std::string rightFilename = "mb/mb_1_0.vtp";
  std::string leftFilename = "mb/mb_2_0.vtp";
  std::string cube = std::string(argv[1]) + "data/" + cubeFilename;
  std::string left = std::string(argv[1]) + "data/" + leftFilename;
  std::string right = std::string(argv[1]) + "data/" + rightFilename;

  // Multiple geometries
  sce.add(std::vector<std::string>{ cube, left, right });

  opt.model.scivis.enable = true;
  opt.model.scivis.array_name = "Normals";

  return TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
           std::string(argv[2]), "TestSDKMultiColoring")
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
