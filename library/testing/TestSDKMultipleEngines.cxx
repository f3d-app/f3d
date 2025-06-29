#include <engine.h>
#include <interactor.h>
#include <log.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include <iostream>

int TestSDKMultipleEngines(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

  f3d::mesh_t mesh{ { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, {}, {}, { 3 }, { 0, 1, 2 } };

  // Test different flags combinations that makes sense
  f3d::engine eng0 = f3d::engine::create();
  eng0.getOptions().model.color.rgb = f3d::color_t{1.0, 0.0, 0.0};
  eng0.getScene().add(mesh);

  f3d::engine eng1 = f3d::engine::create();
  eng1.getOptions().model.color.rgb = f3d::color_t{0.0, 1.0, 0.0};
  eng1.getScene().add(mesh);

  eng0.getInteractor().start();

  return EXIT_SUCCESS;
}
