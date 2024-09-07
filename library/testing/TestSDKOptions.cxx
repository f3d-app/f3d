#include <export.h>
#include <options.h>

#include <iostream>

int TestSDKOptions(int argc, char* argv[])
{
  f3d::options opt;

  // Test bool
  opt.setAsString("model.scivis.cells", "false");
  if (opt.getAsString("model.scivis.cells") != "false")
  {
    std::cerr << "Options setAsString bool with boolalpha is not behaving as expected."
              << std::endl;
    return EXIT_FAILURE;
  }
  opt.setAsString("model.scivis.cells", "1");
  if (opt.getAsString("model.scivis.cells") != "true")
  {
    std::cerr << "Options setAsString bool without boolalpha is not behaving as expected."
              << std::endl;
    return EXIT_FAILURE;
  }
  opt.model.scivis.cells = false;
  if (opt.getAsString("model.scivis.cells") != "false")
  {
    std::cerr << "Options struct with getAsString bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.toggle("model.scivis.cells");
  if (opt.getAsString("model.scivis.cells") != "true")
  {
    std::cerr << "Options toggle is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.isDefault("model.scivis.cells"))
  {
    std::cerr << "Options isDefault bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.reset("model.scivis.cells");
  if (!opt.isDefault("model.scivis.cells"))
  {
    std::cerr << "Options reset bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test int
  opt.setAsString("scene.animation.index", "2");
  if (opt.getAsString("scene.animation.index") != "2")
  {
    std::cerr << "Options setAsString int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.scene.animation.index = 3;
  if (opt.getAsString("scene.animation.index") != "3")
  {
    std::cerr << "Options struct with getAsString int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.isDefault("scene.animation.index"))
  {
    std::cerr << "Options isDefault int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.reset("scene.animation.index");
  if (!opt.isDefault("scene.animation.index"))
  {
    std::cerr << "Options reset int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test double
  opt.setAsString("render.line_width", "2.14");
  if (opt.getAsString("render.line_width") != "2.14")
  {
    std::cerr << "Options setAsString double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.render.line_width = 2.13;
  if (opt.getAsString("render.line_width") != "2.13")
  {
    std::cerr << "Options struct with getAsString double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.isDefault("render.line_width"))
  {
    std::cerr << "Options isDefault double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.reset("render.line_width");
  if (!opt.isDefault("render.line_width"))
  {
    std::cerr << "Options reset double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test ratio_t
  opt.setAsString("scene.animation.speed_factor", "3.17");
  if (opt.getAsString("scene.animation.speed_factor") != "3.17")
  {
    std::cerr << "Options setAsString ratio_t is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.scene.animation.speed_factor = 3.18;
  if (opt.getAsString("scene.animation.speed_factor") != "3.18")
  {
    std::cerr << "Options struct with getAsString ratio_t is not behaving as expected."
              << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.isDefault("scene.animation.speed_factor"))
  {
    std::cerr << "Options isDefault ratio_t is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.reset("scene.animation.speed_factor");
  if (!opt.isDefault("scene.animation.speed_factor"))
  {
    std::cerr << "Options reset ratio_t is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test string
  opt.setAsString("model.color.texture", "testAsString");
  if (opt.getAsString("model.color.texture") != "testAsString")
  {
    std::cerr << "Options setAsString string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.model.color.texture = "testInStruct";
  if (opt.getAsString("model.color.texture") != "testInStruct")
  {
    std::cerr << "Options struct with getAsString string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.isDefault("model.color.texture"))
  {
    std::cerr << "Options isDefault string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.reset("model.color.texture");
  if (!opt.isDefault("model.color.texture"))
  {
    std::cerr << "Options reset string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test double vector
  opt.setAsString("render.background.color", "0.1, 0.2, 0.4");
  if (opt.getAsString("render.background.color") != "0.1, 0.2, 0.4")
  {
    std::cerr << "Options setAsString vector<double> is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.render.background.color = { 0.1, 0.2, 0.5 };
  if (opt.getAsString("render.background.color") != "0.1, 0.2, 0.5")
  {
    std::cerr << "Options struct with getAsString vector<double> is not behaving as expected."
              << std::endl;
    return EXIT_FAILURE;
  }
  if (opt.isDefault("render.background.color"))
  {
    std::cerr << "Options isDefault vector double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.reset("render.background.color");
  if (!opt.isDefault("render.background.color"))
  {
    std::cerr << "Options reset vector double is not behaving as expected." << std::endl;
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
  opt.setAsString("model.scivis.cells", "false").setAsString("model.scivis.cells", "true");
  if (opt.getAsString("model.scivis.cells") != "true")
  {
    std::cerr << "Chaining options as string is not working." << std::endl;
    return EXIT_FAILURE;
  }

  // Test toggle error paths
  try
  {
    opt.toggle("render.line_width");
    std::cerr << "Failing to get an expected exception when toggling non-boolean option."
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    std::cout << "Expected exception: " << ex.what() << std::endl;
  }

  // Test parsing error path
  try
  {
    opt.setAsString("scene.animation.index", "invalid");
    std::cerr << "Failing to get an expected exception when setting an invalid int." << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    std::cout << "Expected exception: " << ex.what() << std::endl;
  }
  try
  {
    opt.setAsString("scene.animation.index", "2147483648");
    std::cerr << "Failing to get an expected exception when setting an out of range int."
              << std::endl;
    std::cerr << "--" << opt.getAsString("scene.animation.index") << "--" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    std::cout << "Expected exception: " << ex.what() << std::endl;
  }

  try
  {
    opt.setAsString("render.line_width", "invalid");
    std::cerr << "Failing to get an expected exception when setting an invalid double."
              << std::endl;
    return EXIT_FAILURE;
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    std::cout << "Expected exception: " << ex.what() << std::endl;
  }
  try
  {
    opt.setAsString("render.line_width",
      "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012"
      "34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234"
      "56789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456"
      "78901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678"
      "90123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"
      "1234567890123456789012345678901234567890");
    return EXIT_FAILURE;
  }
  catch (const f3d::options::parsing_exception& ex)
  {
    std::cout << "Expected exception: " << ex.what() << std::endl;
  }

  // Test copy operator and constructor
  opt.render.line_width = 2.17;
  opt.render.background.color = std::vector<double>({ 0.1, 0.2, 0.7 });
  f3d::options opt2 = opt;
  if (opt2.render.line_width != 2.17)
  {
    std::cerr << "Options copy constructor not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt3;
  opt3 = opt2;
  if (opt3.render.line_width != 2.17)
  {
    std::cerr << "Options copy operator not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt4 = std::move(opt3);
  if (opt4.render.line_width != 2.17)
  {
    std::cerr << "Options move constructor not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt5;
  opt5 = std::move(opt4);
  if (opt5.render.line_width != 2.17)
  {
    std::cerr << "Options move operator not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test getNames
  std::vector<std::string> names = opt.getNames();
  if (names.size() == 0 || names != opt2.getNames())
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
  opt2.render.line_width = 3.12;
  if (opt.isSame(opt2, "render.line_width"))
  {
    std::cerr << "Options isSame not behaving as expected when it should be different."
              << std::endl;
    return EXIT_FAILURE;
  }
  opt2.copy(opt, "render.line_width");
  if (opt2.render.line_width != 2.17)
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
  opt2.render.background.color = { 0.1, 0.2, 0.6 };
  if (opt.isSame(opt2, "render.background.color"))
  {
    std::cerr << "Options isSame not behaving as expected with vectors when it should be different."
              << std::endl;
    return EXIT_FAILURE;
  }
  opt2.copy(opt, "render.background.color");
  if (opt2.render.background.color != std::vector<double>({ 0.1, 0.2, 0.7 }))
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
    std::cout << "Expected exception: " << ex.what() << std::endl;
  }
  try
  {
    opt.copy(opt2, "dummy");
  }
  catch (const f3d::options::inexistent_exception& ex)
  {
    std::cout << "Expected exception: " << ex.what() << std::endl;
  }

  // Test parse
  if (f3d::options::parse<bool>(std::string("true")) != true)
  {
    std::cerr << "Options parse method not behaving as expected with bool." << std::endl;
    return EXIT_FAILURE;
  }
  if (f3d::options::parse<int>(std::string("37")) != 37)
  {
    std::cerr << "Options parse method not behaving as expected with int." << std::endl;
    return EXIT_FAILURE;
  }
  if (f3d::options::parse<double>(std::string("37.2")) != 37.2)
  {
    std::cerr << "Options parse method not behaving as expected with double." << std::endl;
    return EXIT_FAILURE;
  }
  if (f3d::options::parse<f3d::ratio_t>(std::string("0.31")) != 0.31)
  {
    std::cerr << "Options parse method not behaving as expected with ratio_t." << std::endl;
    return EXIT_FAILURE;
  }
  if (f3d::options::parse<std::string>(std::string("  foobar  ")) != "foobar")
  {
    std::cerr << "Options parse method not behaving as expected with string." << std::endl;
    return EXIT_FAILURE;
  }
  if (f3d::options::parse<std::vector<int>>(std::string("1, 2, 3")) !=
    std::vector<int>({ 1, 2, 3 }))
  {
    std::cerr << "Options parse method not behaving as expected with int vector." << std::endl;
    return EXIT_FAILURE;
  }
  if (f3d::options::parse<std::vector<double>>(std::string("0.1, 0.2, 0.3")) !=
    std::vector<double>({ 0.1, 0.2, 0.3 }))
  {
    std::cerr << "Options parse method not behaving as expected with double vector." << std::endl;
    return EXIT_FAILURE;
  }
  if (f3d::options::parse<std::vector<std::string>>(std::string("foo, bar, baz")) !=
    std::vector<std::string>({ "foo", "bar", "baz" }))
  {
    std::cerr << "Options parse method not behaving as expected with string vector." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
