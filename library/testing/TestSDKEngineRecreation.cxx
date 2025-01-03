#include <engine.h>
#include <interactor.h>

int TestSDKEngineRecreation(int argc, char* argv[])
{
  std::string filename = "TestSDKEngineRecreation";
  std::string interactionFilePath = std::string(argv[1]) + "/recordings/" + filename + ".log";
  auto eng = std::make_unique<f3d::engine>(f3d::engine::create(false));
  eng = nullptr;
  for (int i = 0; i < 5; i++)
  {
    auto eng2 = std::make_unique<f3d::engine>(f3d::engine::create(false));
    eng2->getInteractor().playInteraction(interactionFilePath);
  }
  return EXIT_SUCCESS;
}
