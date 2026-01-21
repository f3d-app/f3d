#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <interactor.h>
#include <log.h>
#include <scene.h>
#include <window.h>

#include <iostream>

int TestSDKMultiColoring([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
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

  test("render with multiple colored data",
    TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/",
      std::string(argv[2]), "TestSDKMultiColoring"));
  return test.result();
}
