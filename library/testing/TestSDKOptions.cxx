#include <options.h>

#include <iostream>

int TestSDKOptions(int argc, char* argv[])
{
  f3d::options opt;

  // Test bool
  opt.set("cells", true);
  if (opt.getAsBool("cells") != true)
  {
    std::cerr << "Options set/getAs bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  bool valBool;
  opt.get("cells", valBool);
  if (valBool != true)
  {
    std::cerr << "Options get bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  bool& refBool = opt.getAsBoolRef("cells");
  refBool = false;
  opt.get("cells", valBool);
  if (valBool != false)
  {
    std::cerr << "Options getAsBoolRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test int
  opt.set("animation-index", 1);
  if (opt.getAsInt("animation-index") != 1)
  {
    std::cerr << "Options set/getAs int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  int valInt;
  opt.get("animation-index", valInt);
  if (valInt != 1)
  {
    std::cerr << "Options get int is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  int& refInt = opt.getAsIntRef("animation-index");
  refInt = 2;
  opt.get("animation-index", valInt);
  if (valInt != 2)
  {
    std::cerr << "Options getAsIntRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test double
  opt.set("line-width", 1.7);
  if (opt.getAsDouble("line-width") != 1.7)
  {
    std::cerr << "Options set/getAs double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  double valDouble;
  opt.get("line-width", valDouble);
  if (valDouble != 1.7)
  {
    std::cerr << "Options get double is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  double& refDouble = opt.getAsDoubleRef("line-width");
  refDouble = 2.13;
  opt.get("line-width", valDouble);
  if (valDouble != 2.13)
  {
    std::cerr << "Options getAsDoubleRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test string
  std::string inputString = "test";
  opt.set("texture-base-color", inputString);
  if (opt.getAsString("texture-base-color") != "test")
  {
    std::cerr << "Options set/getAs string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::string valString;
  opt.get("texture-base-color", valString);
  if (valString != "test")
  {
    std::cerr << "Options get string is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  inputString = "testChar";
  opt.set("texture-base-color", inputString.c_str());
  if (opt.getAsString("texture-base-color") != "testChar")
  {
    std::cerr << "Options set char* is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::string& refString = opt.getAsStringRef("texture-base-color");
  refString = "dummy";
  opt.get("texture-base-color", valString);
  if (valString != "dummy")
  {
    std::cerr << "Options getAsStringRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // XXX Test int vector is not done as their is currently no int vector option

  // Test double vector
  opt.set("background-color", { 0.1, 0.2, 0.3 });
  if (opt.getAsDoubleVector("background-color") != std::vector<double>{ 0.1, 0.2, 0.3 })
  {
    std::cerr << "Options getAsDoubleVector is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::vector<double> valDoubleVec;
  opt.get("background-color", valDoubleVec);
  if (valDoubleVec != std::vector<double>{ 0.1, 0.2, 0.3 })
  {
    std::cerr << "Options get double vector is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.set("background-color", std::vector<double>{ 0.1, 0.2, 0.4 });
  if (opt.getAsDoubleVectorRef("background-color") != std::vector<double>{ 0.1, 0.2, 0.4 })
  {
    std::cerr << "Options set double vector is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::vector<double>& refDoubleVec = opt.getAsDoubleVectorRef("background-color");
  refDoubleVec = std::vector<double>{ 0.1, 0.2, 0.5 };
  opt.get("background-color", valDoubleVec);
  if (valDoubleVec != std::vector<double>{ 0.1, 0.2, 0.5 })
  {
    std::cerr << "Options getAsDoubleVectorRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test chaining options
  opt.set("cells", true).set("cells", false);
  if (opt.getAsBool("cells") != false)
  {
    std::cerr << "Chaining options is not working." << std::endl;
    return EXIT_FAILURE;
  }

  // Test error paths
  double val;
  opt.set("cells", 2.13);
  opt.get("cells", val);
  try
  {
    double& refVal = opt.getAsDoubleRef("cells");
  }
  catch (const f3d::options::exception& ex)
  {
    std::cout << "Expected exception:" << ex.what() << std::endl;
  }

  opt.set("dummy", 2.13);
  opt.get("dummy", val);
  try
  {
    double& refVal = opt.getAsDoubleRef("dummy");
  }
  catch (const f3d::options::exception& ex)
  {
    std::cout << "Expected exception:" << ex.what() << std::endl;
  }

  if (opt.getAsBool("cells") != false)
  {
    std::cerr << "Options error paths not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test copy operator and constructor
  f3d::options opt2 = opt;
  if (opt2.getAsDouble("line-width") != 2.13)
  {
    std::cerr << "Options copy constructor not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt3;
  opt3 = opt2;
  if (opt3.getAsDouble("line-width") != 2.13)
  {
    std::cerr << "Options copy operator not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt4 = std::move(opt3);
  if (opt4.getAsDouble("line-width") != 2.13)
  {
    std::cerr << "Options move constructor not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt5;
  opt5 = std::move(opt4);
  if (opt5.getAsDouble("line-width") != 2.13)
  {
    std::cerr << "Options move operator not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
