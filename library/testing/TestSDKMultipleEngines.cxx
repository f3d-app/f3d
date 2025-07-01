#include <engine.h>
#include <interactor.h>
#include <log.h>
#include <options.h>
#include <scene.h>
#include <window.h>

#include <iostream>
#include <thread>

void RunEngine(int index)
{
  f3d::mesh_t mesh{ { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f }, {}, {}, { 3 }, { 0, 1, 2 } };

  // Test different flags combinations that makes sense
  f3d::engine eng = f3d::engine::create();
  eng.getOptions().model.color.rgb =
    index == 0 ? f3d::color_t{ 1.0, 0.0, 0.0 } : f3d::color_t{ 0.0, 1.0, 0.0 };
  eng.getScene().add(mesh);

  eng.getInteractor().start();
}

int TestSDKMultipleEngines(int argc, char* argv[])
{
  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

  std::thread t0(RunEngine, 0);
  std::thread t1(RunEngine, 1);

  t0.join();
  t1.join();

  return EXIT_SUCCESS;
}
