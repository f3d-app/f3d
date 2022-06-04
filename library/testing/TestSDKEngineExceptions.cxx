#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <window.h>

#include <iostream>

int TestSDKEngineExceptions(int argc, char* argv[])
{
  f3d::engine eng(f3d::window::WindowType::NO_RENDER);

  try
  {
    f3d::window& win = eng.getWindow();
    std::cerr << "An exception has not been thrown when getting a non-existent window" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::engine::exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  try
  {
    f3d::interactor& inter = eng.getInteractor();
    std::cerr << "An exception has not been thrown when getting a non-existent interactor"
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::engine::exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  return EXIT_SUCCESS;
}
