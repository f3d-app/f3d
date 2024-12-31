#include <engine.h>
#include <interactor.h>

#include <iostream>

int TestSDKEngineRecreation(int argc, char* argv[])
{
  std::string filename = "TestSDKEngineRecreation";
  std::string interactionFilePath = std::string(argv[1]) + "/recordings/" + filename + ".log";
  f3d::engine* eng = new f3d::engine(f3d::engine::create(false));
  eng->getInteractor().start(1 / 30, [eng]() { eng->getInteractor().stop(); });
  delete eng;
  for (int i = 0; i < 5; i++)
  {
    f3d::engine* eng2 = new f3d::engine(f3d::engine::create(false));
    eng2->getInteractor().playInteraction(interactionFilePath);
    delete eng2;
  }
  return EXIT_SUCCESS;
}
