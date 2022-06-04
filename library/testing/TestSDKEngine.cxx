#include <engine.h>
#include <interactor.h>
#include <loader.h>
#include <options.h>
#include <window.h>

#include <iostream>

int TestSDKEngine(int argc, char* argv[])
{
  // Test different flags combinations that makes sense
  f3d::engine eng0(f3d::window::WindowType::NO_RENDER);
  f3d::loader& load = eng0.getLoader();

  f3d::engine eng1(f3d::window::WindowType::NATIVE);
  f3d::loader& load1 = eng1.getLoader();
  f3d::window& window1 = eng1.getWindow();
  f3d::interactor& inter1 = eng1.getInteractor();

  f3d::engine eng2(f3d::window::WindowType::NATIVE_OFFSCREEN);
  f3d::loader& load2 = eng2.getLoader();
  f3d::window& window2 = eng2.getWindow();
  f3d::interactor& inter2 = eng2.getInteractor();

  f3d::engine eng3(f3d::window::WindowType::EXTERNAL);
  f3d::loader& load3 = eng3.getLoader();
  f3d::window& window3 = eng3.getWindow();
//  f3d::interactor& inter3 = eng3.getInteractor(); TODO add support for interactor and external window

  // Test option setters
  f3d::options opt;
  opt.set("quiet", true);

  eng0.setOptions(opt);
  if (!eng0.getOptions().getAsBool("quiet"))
  {
    std::cerr << "Unexpected options value using f3d::engine::setOptions(const options& opt)"
              << std::endl;
    return EXIT_FAILURE;
  }

  opt.set("line-width", 1.7);
  eng0.setOptions(std::move(opt));
  if (eng0.getOptions().getAsDouble("line-width") != 1.7)
  {
    std::cerr << "Unexpected options value using f3d::engine::setOptions(options&& opt)"
              << std::endl;
    return EXIT_FAILURE;
  }

  // Test static information methods
  auto libInfo = f3d::engine::getLibInfo();
  if (libInfo.find("VTK version") == libInfo.end())
  {
    std::cerr << "Unexpected libInfo output" << std::endl;
    return EXIT_FAILURE;
  }

  auto readersInfo = f3d::engine::getReadersInfo();
  if (readersInfo.size() == 0)
  {
    std::cerr << "Unexpected readersInfo output" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
