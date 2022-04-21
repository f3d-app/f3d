#include <f3d_engine.h>
#include <f3d_interactor.h>
#include <f3d_loader.h>
#include <f3d_window.h>

#include <iostream>

int TestSDKEngineExceptions(int argc, char* argv[])
{
  f3d::engine eng(f3d::engine::FLAGS_NONE);
  
  try
  {
    f3d::window& win = eng.getWindow();
  }
  catch (const f3d::engine::window_exception& ex)
  {
    std::cout << ex.what() << std::endl;
  }
 
  try
  {
    f3d::interactor& inter = eng.getInteractor();
  }
  catch (const f3d::engine::interactor_exception& ex)
  {
    std::cout << ex.what() << std::endl;
    exit(EXIT_SUCCESS);
  }

  return EXIT_FAILURE;
}
