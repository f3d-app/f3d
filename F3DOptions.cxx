#include "F3DOptions.h"

#include "vtkF3DGenericImporter.h"

//----------------------------------------------------------------------------
void F3DOptions::InitializeFromArgs(int argc, char** argv)
{
  try
  {
    cxxopts::Options options(argv[0], f3d::AppTitle);
    options
        .positional_help("input_file")
        .show_positional_help();

    options
        .add_options()("n,normals", "Show mesh normals", cxxopts::value<bool>(this->Normals))("i,input", "Input file", cxxopts::value<std::string>(), "file")("h,help", "Print help")("x,axis", "Show axis", cxxopts::value<bool>(this->Axis))("g,grid", "Show grid", cxxopts::value<bool>(this->Grid))("s,size", "Window size", cxxopts::value<std::vector<int>>(this->WindowSize)->default_value("1000,600"));

    options.parse_positional({"input", "positional"});

    if (argc == 1)
    {
      std::cout << options.help() << std::endl;
      exit(EXIT_FAILURE);
    }

    auto result = options.parse(argc, argv);

    if (result.count("help") > 0)
    {
      std::cout << options.help() << std::endl;
      exit(EXIT_SUCCESS);
    }

    this->Input = result["input"].as<std::string>().c_str();
  }
  catch (const cxxopts::OptionException &e)
  {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

//----------------------------------------------------------------------------
void F3DOptions::InitializeFromFile(const std::string &fname)
{

}
