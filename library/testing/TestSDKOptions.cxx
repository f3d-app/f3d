#include <options.h>

#include <iostream>

int TestSDKOptions(int argc, char* argv[])
{
  f3d::options opt;

  // Test bool
  opt.set("model.scivis.cells", true);
  if (opt.getAsBool("model.scivis.cells") != true)
  {
    std::cerr << "Options set/getAs bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  bool valBool;
  opt.get("model.scivis.cells", valBool);
  if (valBool != true)
  {
    std::cerr << "Options get bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  bool& refBool = opt.getAsBoolRef("model.scivis.cells");
  refBool = false;
  opt.get("model.scivis.cells", valBool);
  if (valBool != false)
  {
    std::cerr << "Options getAsBoolRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test int
  opt.set("scene.animation.index", 1);
  if (opt.getAsInt("scene.animation.index") != 1)
  {
    std::cerr << "Options set/getAs int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  int valInt;
  opt.get("scene.animation.index", valInt);
  if (valInt != 1)
  {
    std::cerr << "Options get int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  int& refInt = opt.getAsIntRef("scene.animation.index");
  refInt = 2;
  opt.get("scene.animation.index", valInt);
  if (valInt != 2)
  {
    std::cerr << "Options getAsIntRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test double
  opt.set("render.line-width", 1.7);
  if (opt.getAsDouble("render.line-width") != 1.7)
  {
    std::cerr << "Options set/getAs double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  double valDouble;
  opt.get("render.line-width", valDouble);
  if (valDouble != 1.7)
  {
    std::cerr << "Options get double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  double& refDouble = opt.getAsDoubleRef("render.line-width");
  refDouble = 2.13;
  opt.get("render.line-width", valDouble);
  if (valDouble != 2.13)
  {
    std::cerr << "Options getAsDoubleRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test string
  std::string inputString = "test";
  opt.set("model.color.texture", inputString);
  if (opt.getAsString("model.color.texture") != "test")
  {
    std::cerr << "Options set/getAs string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::string valString;
  opt.get("model.color.texture", valString);
  if (valString != "test")
  {
    std::cerr << "Options get string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  inputString = "testChar";
  opt.set("model.color.texture", inputString.c_str());
  if (opt.getAsString("model.color.texture") != "testChar")
  {
    std::cerr << "Options set char* is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::string& refString = opt.getAsStringRef("model.color.texture");
  refString = "dummy";
  opt.get("model.color.texture", valString);
  if (valString != "dummy")
  {
    std::cerr << "Options getAsStringRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // XXX Test int vector is not done as their is currently no int vector option

  // Test double vector
  opt.set("render.background.color", { 0.1, 0.2, 0.3 });
  if (opt.getAsDoubleVector("render.background.color") != std::vector<double>{ 0.1, 0.2, 0.3 })
  {
    std::cerr << "Options getAsDoubleVector is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::vector<double> valDoubleVec;
  opt.get("render.background.color", valDoubleVec);
  if (valDoubleVec != std::vector<double>{ 0.1, 0.2, 0.3 })
  {
    std::cerr << "Options get double vector is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.set("render.background.color", std::vector<double>{ 0.1, 0.2, 0.4 });
  if (opt.getAsDoubleVectorRef("render.background.color") != std::vector<double>{ 0.1, 0.2, 0.4 })
  {
    std::cerr << "Options set double vector is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::vector<double>& refDoubleVec = opt.getAsDoubleVectorRef("render.background.color");
  refDoubleVec = std::vector<double>{ 0.1, 0.2, 0.5 };
  opt.get("render.background.color", valDoubleVec);
  if (valDoubleVec != std::vector<double>{ 0.1, 0.2, 0.5 })
  {
    std::cerr << "Options getAsDoubleVectorRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test chaining options
  opt.set("model.scivis.cells", true).set("model.scivis.cells", false);
  if (opt.getAsBool("model.scivis.cells") != false)
  {
    std::cerr << "Chaining options is not working." << std::endl;
    return EXIT_FAILURE;
  }

  // Test error paths
  double val;
  opt.set("model.scivis.cells", 2.13);
  opt.get("model.scivis.cells", val);
  try
  {
    double& refVal = opt.getAsDoubleRef("model.scivis.cells");
  }
  catch (const f3d::options::incompatible_exception& ex)
  {
    std::cout << "Expected exception:" << ex.what() << std::endl;
  }

  opt.set("dummy", 2.13);
  opt.get("dummy", val);
  try
  {
    double& refVal = opt.getAsDoubleRef("dummy");
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

  // Test copy operator and constructor
  f3d::options opt2 = opt;
  if (opt2.getAsDouble("render.line-width") != 2.13)
  {
    std::cerr << "Options copy constructor not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt3;
  opt3 = opt2;
  if (opt3.getAsDouble("render.line-width") != 2.13)
  {
    std::cerr << "Options copy operator not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt4 = std::move(opt3);
  if (opt4.getAsDouble("render.line-width") != 2.13)
  {
    std::cerr << "Options move constructor not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt5;
  opt5 = std::move(opt4);
  if (opt5.getAsDouble("render.line-width") != 2.13)
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
  if (!opt.isSame(opt2, "render.line-width"))
  {
    std::cerr << "Options isSame not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt2.set("render.line-width", 3.12);
  if (opt.isSame(opt2, "render.line-width"))
  {
    std::cerr << "Options isSame not behaving as expected when it should be different."
              << std::endl;
    return EXIT_FAILURE;
  }
  opt2.copy(opt, "render.line-width");
  if (opt2.getAsDouble("render.line-width") != 2.13)
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
  opt2.set("render.background.color", { 0.1, 0.2, 0.6 });
  if (opt.isSame(opt2, "render.background.color"))
  {
    std::cerr << "Options isSame not behaving as expected with vectors when it should be different."
              << std::endl;
    return EXIT_FAILURE;
  }
  opt2.copy(opt, "render.background.color");
  if (opt2.getAsDoubleVector("render.background.color") != std::vector<double>{ 0.1, 0.2, 0.5 })
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
