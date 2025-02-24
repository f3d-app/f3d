#include "PseudoUnitTest.h"

#include <engine.h>
#include <interactor.h>
#include <scene.h>

int TestCameraOrbit(int argc, char* argv[])
{
  PseudoUnitTest test;
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::interactor& inter = eng.getInteractor();
  sce.add(std::string(argv[1]) + "/data/f3d.glb");

  // Set camera orbit time
  eng.getOptions().scene.camera.orbit_time = 5.0; // 5 seconds for a full orbit

  inter.startAnimation();
  test("isPlaying after start", inter.isPlayingAnimation());

  // Simulate some time passing
  sce.loadAnimationTime(2.5); // Halfway through the orbit

  // Check camera position
  vtkCamera* camera =
    sce.getWindow().getRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
  double* position = camera->GetPosition();
  test("camera x position", std::abs(position[0] - 0.0) < 0.1);  // Expect near zero x position
  test("camera y position", std::abs(position[1] - 10.0) < 0.1); // Expect near radius y position

  inter.stopAnimation();
  test("isPlaying after stop", !inter.isPlayingAnimation());

  return test.result();
} 