#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <interactor.h>
#include <log.h>
#include <mesh_view.h>
#include <scene.h>
#include <window.h>

int TestSDKSceneFromMemoryZeroCopyExceptions(
  [[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();

  class NoPointMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      return { .pointCount = 0 };
    }
  };

  test.expect<f3d::scene::load_failure_exception>(
    "add with no point", [&]() { sce.add(std::make_shared<NoPointMesh>()); });

  return test.result();
}
