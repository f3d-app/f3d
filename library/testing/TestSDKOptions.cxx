#include <export.h>
#include <options.h>

#include <iostream>

int TestSDKOptions(int argc, char* argv[])
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
  opt.setAsString("model.scivis.cells", "false");
  if (opt.getAsString("model.scivis.cells") != "false")
  {
    std::cerr << "Options setAsString bool with boolalpha  is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.setAsString("model.scivis.cells", "1");
  if (opt.getAsString("model.scivis.cells") != "true")
  {
    std::cerr << "Options setAsString bool without boolalpha  is not behaving as expected." << std::endl;
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
  opt.setAsString("scene.animation.index", "2");
  if (opt.getAsString("scene.animation.index") != "2")
  {
    std::cerr << "Options setAsString int is not behaving as expected." << std::endl;
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
  opt.setAsString("render.line_width", "2.13");
  if (opt.getAsString("render.line_width") != "2.13")
  {
    std::cerr << "Options setAsString double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test ratio_t
  opt.set("scene.animation.speed_factor", f3d::ratio_t(3.17));
  if (std::get<f3d::ratio_t>(opt.get("scene.animation.speed_factor")) != 3.17)
  {
    std::cerr << "Options set/get ratio_t is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.getAsString("scene.animation.speed_factor") != "3.17")
  {
    std::cerr << "Options getAsString ratio_t is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.setAsString("scene.animation.speed_factor", "3.17");
  if (opt.getAsString("scene.animation.speed_factor") != "3.17")
  {
    std::cerr << "Options setAsString ratio_t is not behaving as expected." << std::endl;
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
  opt.setAsString("model.color.texture", "testAsString");
  if (opt.getAsString("model.color.texture") != "testAsString")
  {
    std::cerr << "Options setAsString string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test double vector
  opt.set("render.background.color", std::vector<double>{ 0.1, 0.2, 0.3 });
  if (std::get<std::vector<double>>(opt.get("render.background.color")) != std::vector<double>{ 0.1, 0.2, 0.3 })
  {
    std::cerr << "Options set/get vector<double> is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.getAsString("render.background.color") != "0.1, 0.2, 0.3")
  {
    std::cerr << "Options getAsString vector<double> is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.setAsString("render.background.color", "0.1, 0.2, 0.4");
  if (opt.getAsString("render.background.color") != "0.1, 0.2, 0.4")
  {
    std::cerr << "Options setAsString vector<double> is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test closest option
  auto closest = opt.getClosestOption("modle.sciivs.cell");
  if (closest.first != "model.scivis.cells" || closest.second != 5)
  {
    std::cerr << "Failed to get the closest option." << std::endl;
    return EXIT_FAILURE;
  }

  closest = opt.getClosestOption("model.scivis.cells");
  if (closest.first != "model.scivis.cells" || closest.second != 0)
  {
    std::cerr << "Failed to get the exact option." << std::endl;
    return EXIT_FAILURE;
  }

  // Test chaining options
  opt.set("model.scivis.cells", true).set("model.scivis.cells", false);
  if (std::get<bool>(opt.get("model.scivis.cells")) != false)
  {
    std::cerr << "Chaining options is not working." << std::endl;
    return EXIT_FAILURE;
  }

  /*
  // Test error paths
  double val;
  opt.set("model.scivis.cells", 2.13);
  opt.get("model.scivis.cells", val);
  try
  {
    const double& refVal = opt.getAsDoubleRef("model.scivis.cells");
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    std::cout << "Expected exception:" << ex.what() << std::endl;
  }

  opt.set("dummy", 2.13);
  opt.get("dummy", val);
  try
  {
    const double& refVal = opt.getAsDoubleRef("dummy");
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    std::cout << "Expected exception:" << ex.what() << std::endl;
  }

  if (opt.getAsBool("model.scivis.cells") != false)
  {
    std::cerr << "Options error paths not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  */

  // Test copy operator and constructor
  f3d::options opt2 = opt;
  if (std::get<double>(opt2.get("render.line_width")) != 2.13)
  {
    std::cerr << "Options copy constructor not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt3;
  opt3 = opt2;
  if (std::get<double>(opt3.get("render.line_width")) != 2.13)
  {
    std::cerr << "Options copy operator not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt4 = std::move(opt3);
  if (std::get<double>(opt4.get("render.line_width")) != 2.13)
  {
    std::cerr << "Options move constructor not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt5;
  opt5 = std::move(opt4);
  if (std::get<double>(opt5.get("render.line_width")) != 2.13)
  {
    std::cerr << "Options move operator not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test getNames
  auto names = opt.getNames();
  if (names.size() == 0 || opt.getNames() != opt2.getNames())
  {
    std::cerr << "Options getNames not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test isSame/copy
  if (!opt.isSame(opt2, "render.line_width"))
  {
    std::cerr << "Options isSame not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt2.set("render.line_width", 3.12);
  if (opt.isSame(opt2, "render.line_width"))
  {
    std::cerr << "Options isSame not behaving as expected when it should be different."
              << std::endl;
    return EXIT_FAILURE;
  }
  opt2.copy(opt, "render.line_width");
  if (std::get<double>(opt2.get("render.line_width")) != 2.13)
  {
    std::cerr << "Options copy method not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test isSame/copy vector
  if (!opt.isSame(opt2, "render.background.color"))
  {
    std::cerr << "Options isSame not behaving as expected with vectors." << std::endl;
    return EXIT_FAILURE;
  }
  opt2.set("render.background.color", std::vector<double>{ 0.1, 0.2, 0.6 });
  if (opt.isSame(opt2, "render.background.color"))
  {
    std::cerr << "Options isSame not behaving as expected with vectors when it should be different."
              << std::endl;
    return EXIT_FAILURE;
  }
  opt2.copy(opt, "render.background.color");
  if (std::get<std::vector<double>>(opt2.get("render.background.color")) != std::vector<double>{ 0.1, 0.2, 0.4 })
  {
    std::cerr << "Options copy method not behaving as expected with vectors." << std::endl;
    return EXIT_FAILURE;
  }

  // Test isSame/copy error path
  try
  {
    opt.isSame(opt2, "dummy");
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    std::cout << "Expected exception:" << ex.what() << std::endl;
  }
  try
  {
    opt.copy(opt2, "dummy");
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    std::cout << "Expected exception:" << ex.what() << std::endl;
  }

  return EXIT_SUCCESS;
}
