#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <window.h>

#include <iostream>

int TestSDKEngineExceptions(int argc, char* argv[])
{
  f3d::engine eng(f3d::engine::FLAGS_NONE);

  try
  {
    f3d::window& win = eng.getWindow();
  }
  catch (const f3d::engine::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  try
  {
    f3d::interactor& inter = eng.getInteractor();
  }
  catch (const f3d::engine::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
    exit(EXIT_SUCCESS);
  }

  return EXIT_FAILURE;
}
