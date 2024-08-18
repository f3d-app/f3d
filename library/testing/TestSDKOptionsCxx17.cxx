#include <export.h>
#include <options.h>

#include <iostream>

int TestSDKOptionsCxx17(int argc, char* argv[])
{
  f3d::options opt;

  // Test bool
  opt.set("model.scivis.cells", true);
  if (std::get<bool>(opt.get("model.scivis.cells")) != true)
  {
    std::cerr << "Options set/get bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.getAsString("model.scivis.cells") != "true")
  {
    std::cerr << "Options getAsString bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test int
  opt.set("scene.animation.index", 1);
  if (std::get<int>(opt.get("scene.animation.index")) != 1)
  {
    std::cerr << "Options set/get int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.getAsString("scene.animation.index") != "1")
  {
    std::cerr << "Options getAsString int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test double
  opt.set("render.line_width", 1.7);
  if (std::get<double>(opt.get("render.line_width")) != 1.7)
  {
    std::cerr << "Options set/get double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.getAsString("render.line_width") != "1.7")
  {
    std::cerr << "Options getAsString double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test ratio_t
  opt.set("scene.animation.speed_factor", 3.17);
  if (std::get<double>(opt.get("scene.animation.speed_factor")) != 3.17)
  {
    std::cerr << "Options set/get ratio_t is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.getAsString("scene.animation.speed_factor") != "3.17")
  {
    std::cerr << "Options getAsString ratio_t is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test string
  std::string inputString = "test";
  opt.set("model.color.texture", inputString);
  if (std::get<std::string>(opt.get("model.color.texture")) != "test")
  {
    std::cerr << "Options set/get string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.getAsString("model.color.texture") != "test")
  {
    std::cerr << "Options getAsString string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test double vector
  opt.set("render.background.color", std::vector<double>{ 0.1, 0.2, 0.3 });
  if (std::get<std::vector<double>>(opt.get("render.background.color")) !=
    std::vector<double>{ 0.1, 0.2, 0.3 })
  {
    std::cerr << "Options set/get vector<double> is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.getAsString("render.background.color") != "0.1, 0.2, 0.3")
  {
    std::cerr << "Options getAsString vector<double> is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test chaining options
  opt.set("model.scivis.cells", true).set("model.scivis.cells", false);
  if (std::get<bool>(opt.get("model.scivis.cells")) != false)
  {
    std::cerr << "Chaining options is not working." << std::endl;
    return EXIT_FAILURE;
  }

  // Test error paths
  try
  {
    opt.set("model.scivis.cells", 2.13);
    std::cerr << "Failing to get an expected exception with setting an incompatible value."
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    std::cout << "Expected exception: " << ex.what() << std::endl;
  }

  try
  {
    opt.set("dummy", 2.13);
    std::cerr << "Failing to get an expected exception with setting an inexistent option."
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    std::cout << "expected exception: " << ex.what() << std::endl;
  }

  try
  {
    opt.get("dummy");
    std::cerr << "Failing to get an expected exception with getting an inexistent option."
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    std::cout << "expected exception: " << ex.what() << std::endl;
  }

  return EXIT_SUCCESS;
}
