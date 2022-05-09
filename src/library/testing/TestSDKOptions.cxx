#include <f3d_options.h>

#include <iostream>

int TestSDKOptions(int argc, char* argv[])
{
  f3d::options opt;

  // Test bool
  opt.set("quiet", true);
  if (opt.getAsBool("quiet") != true)
  {
    std::cerr << "Options set/getAs bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  bool valBool;
  opt.get("quiet", valBool);
  if (valBool != true)
  {
    std::cerr << "Options get bool is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  bool& refBool = opt.getAsBoolRef("quiet");
  refBool = false;
  opt.get("quiet", valBool);
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
  opt.set("texture-base-color", "test");
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
  valString = "testChar";
  opt.set("texture-base-color", valString.c_str());
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

  // Test int vector
  opt.set("resolution", { 1000, 600 });
  if (opt.getAsIntVector("resolution") != std::vector<int>{ 1000, 600 })
  {
    std::cerr << "Options getAsIntVector is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::vector<int> valIntVec;
  opt.get("resolution", valIntVec);
  if (valIntVec != std::vector<int>{ 1000, 600 })
  {
    std::cerr << "Options get int vector is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  opt.set("resolution", std::vector<int>{ 1000, 700 });
  if (opt.getAsIntVectorRef("resolution") != std::vector<int>{ 1000, 700 })
  {
    std::cerr << "Options set int vector is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }
  std::vector<int>& refIntVec = opt.getAsIntVectorRef("resolution");
  refIntVec = std::vector<int>{ 1000, 800 };
  opt.get("resolution", valIntVec);
  if (valIntVec != std::vector<int>{ 1000, 800 })
  {
    std::cerr << "Options getAsIntVectorRef is not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

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

  // Test error paths
  double val;
  opt.set("quiet", 2.13);
  opt.get("quiet", val);
  try
  {
    double& refVal = opt.getAsDoubleRef("quiet");
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

  if (opt.getAsBool("quiet") != false)
  {
    std::cerr << "Options error paths not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  // Test copy operator and constructor
  f3d::options opt2(opt);
  if (opt2.getAsDouble("line-width") != 2.13)
  {
    std::cerr << "Options copy constructor not behaving as expected." << std::endl;
    return EXIT_FAILURE;
  }

  f3d::options opt3 = opt2;
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
