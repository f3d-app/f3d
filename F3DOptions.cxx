#include "F3DOptions.h"

#include "vtkF3DGenericImporter.h"

#include <vtk_jsoncpp.h>

#include <fstream>

//----------------------------------------------------------------------------
bool F3DOptions::InitializeFromArgs(int argc, char** argv)
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
  return true;
}

//----------------------------------------------------------------------------
bool F3DOptions::InitializeFromFile(const std::string &fname)
{
  std::ifstream file;
  file.open(fname.c_str());

  if (!file.is_open())
  {
    std::cerr << "Unable to open configuration file " << fname << endl;
    return false;
  }
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  Json::Value root;
  std::string errs;
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  bool success = Json::parseFromStream(builder, file, &root, &errs);
  if (!success)
  {
    std::cerr << "Unable to parse the configuration file " << fname << endl;
    return false;
  }

  // TODO

  return true;
}
