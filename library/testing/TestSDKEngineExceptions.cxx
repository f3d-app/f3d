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

// These tests are defined for coverage
#ifdef __linux__
  try
  {
    eng = f3d::engine::createWGL();
    std::cerr << "An exception has not been thrown when creating a WGL engine on Linux"
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::context::loading_exception& ex)
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

  try
  {
    eng = f3d::engine::createExternalCOCOA();
    std::cerr << "An exception has not been thrown when creating an external COCOA engine on Linux"
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::context::loading_exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  try
  {
    eng = f3d::engine::createExternal(f3d::context::getSymbol("invalid", "invalid"));
    std::cerr << "An exception has not been thrown when loading an invalid library" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::context::loading_exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  try
  {
    eng = f3d::engine::createExternal(f3d::context::getSymbol("GLX", "invalid"));
    std::cerr << "An exception has not been thrown when loading an invalid symbol" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::context::symbol_exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  // cover operator=(engine&&)
  eng = f3d::engine::create(false);
#endif

  return EXIT_SUCCESS;
}
