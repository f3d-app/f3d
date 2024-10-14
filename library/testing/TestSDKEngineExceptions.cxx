#include <engine.h>
#include <interactor.h>
#include <window.h>

#include <iostream>

int TestSDKEngineExceptions(int argc, char* argv[])
{
  f3d::engine eng = f3d::engine::createNone();

  try
  {
    const f3d::window& win = eng.getWindow();
    std::cerr << "An exception has not been thrown when getting a non-existent window" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::engine::no_window_exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  try
  {
    const f3d::interactor& inter = eng.getInteractor();
    std::cerr << "An exception has not been thrown when getting a non-existent interactor"
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::engine::no_interactor_exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }

// These tests are defined for coverage of WGL APIs
#ifdef __linux__
  try
  {
    eng = f3d::engine::createWGL();
    std::cerr << "An exception has not been thrown when creating a WGL engine on Linux"
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::engine::no_window_exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  try
  {
    eng = f3d::engine::createExternalWGL();
    std::cerr << "An exception has not been thrown when creating an external WGL engine on Linux"
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::context::loading_exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }
#endif

  return EXIT_SUCCESS;
}
