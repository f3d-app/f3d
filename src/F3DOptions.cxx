#include "F3DOptions.h"

#include "F3DLog.h"

#include "vtkF3DGenericImporter.h"

#include <vtk_jsoncpp.h>

#include <fstream>

//----------------------------------------------------------------------------
bool F3DOptions::InitializeFromArgs(int argc, char** argv)
{
  try
  {
    cxxopts::Options options(f3d::AppName, f3d::AppTitle);
    options
      .positional_help("input_file")
      .show_positional_help();

    options
      .add_options()
      ("input", "Input file", cxxopts::value<std::string>(), "<file>")
      ("o,output", "Render to file", cxxopts::value<std::string>(this->Output), "<png file>")
      ("h,help", "Print help")
      ("version", "Print version details")
      ("v,verbose", "Enable verbose mode", cxxopts::value<bool>(this->Verbose))
      ("x,axis", "Show axis", cxxopts::value<bool>(this->Axis))
      ("g,grid", "Show grid", cxxopts::value<bool>(this->Grid))
      ("p,progress", "Show progress bar", cxxopts::value<bool>(this->Progress))
      ("m,geometry-only", "Do not read materials, cameras and lights from file", cxxopts::value<bool>(this->GeometryOnly));

    options
      .add_options("Material")
      ("e,edges", "Show cell edges", cxxopts::value<bool>(this->Edges))
      ("point-size", "Point size", cxxopts::value<double>(this->PointSize)->default_value("10.0"), "<size>")
      ("color", "Solid color", cxxopts::value<std::vector<double>>(this->SolidColor)->default_value("1.0,1.0,1.0"), "<R,G,B>")
      ("opacity", "Opacity", cxxopts::value<double>(this->Opacity)->default_value("1.0"), "<opacity>")
      ("roughness", "Roughness coefficient (0.0-1.0)", cxxopts::value<double>(this->Roughness)->default_value("0.3"), "<roughness>")
      ("metallic", "Metallic coefficient (0.0-1.0)", cxxopts::value<double>(this->Metallic)->default_value("0.0"), "<metallic>");

    options
      .add_options("Window")
      ("bg-color", "Background color", cxxopts::value<std::vector<double>>(this->BackgroundColor)->default_value("0.2,0.2,0.2"), "<R,G,B>")
      ("resolution", "Window resolution", cxxopts::value<std::vector<int>>(this->WindowSize)->default_value("1000,600"), "<width,height>")
      ("t,timer", "Display frame per second", cxxopts::value<bool>(this->FPS));

    options
      .add_options("Scientific visualization")
      ("scalars", "Color by scalars", cxxopts::value<std::string>(this->Scalars)->implicit_value("f3d_reserved"), "<array_name>")
      ("comp", "Specify the component used", cxxopts::value<int>(this->Component), "<comp_index>")
      ("c,cells", "The array is located on cells", cxxopts::value<bool>(this->Cells))
      ("range", "Custom range for the array", cxxopts::value<std::vector<double>>(this->Range), "<min,max>")
      ("b,bar", "Show scalar bar", cxxopts::value<bool>(this->Bar));

#if F3D_HAS_RAYTRACING
    options
      .add_options("Raytracing")
      ("r,raytracing", "Enable raytracing", cxxopts::value<bool>(this->Raytracing))
      ("samples", "Number of samples per pixel", cxxopts::value<int>(this->Samples)->default_value("5"), "<samples>")
      ("s,denoise", "Denoise the image", cxxopts::value<bool>(this->Denoise));
#endif

    options
      .add_options("PostFX (OpenGL)")
      ("d,depth-peeling", "Enable depth peeling", cxxopts::value<bool>(this->DepthPeeling))
      ("u,ssao", "Enable Screen-Space Ambient Occlusion", cxxopts::value<bool>(this->SSAO))
      ("f,fxaa", "Enable Fast Approximate Anti-Aliasing", cxxopts::value<bool>(this->FXAA));

    options
      .add_options("Testing")
      ("ref", "Reference image (for testing)", cxxopts::value<std::string>(this->Reference), "<png file>")
      ("ref-threshold", "Testing threshold", cxxopts::value<double>(this->RefThreshold), "<threshold>");

    options.parse_positional({"input", "positional"});

    if (argc == 1)
    {
      F3DLog::Print(F3DLog::Severity::Info, options.help());
      exit(EXIT_FAILURE);
    }

    auto result = options.parse(argc, argv);

    if (result.count("help") > 0)
    {
      F3DLog::Print(F3DLog::Severity::Info, options.help());
      exit(EXIT_SUCCESS);
    }

    if (result.count("version") > 0)
    {
      std::string version = f3d::AppTitle;
      version += "\nVersion: ";
      version += f3d::AppVersion;
      version += "\nBuild date: ";
      version += f3d::AppBuildDate;
      version += "\nRayTracing module: ";
#if F3D_HAS_RAYTRACING
      version += "ON";
#else
      version += "OFF";
#endif
      version += "\nAuthor: Kitware SAS";

      F3DLog::Print(F3DLog::Severity::Info, version);
      exit(EXIT_SUCCESS);
    }

    this->Input = result["input"].as<std::string>().c_str();
  }
  catch (const cxxopts::OptionException &e)
  {
    F3DLog::Print(F3DLog::Severity::Error, "Error parsing options: ", e.what());
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
    F3DLog::Print(F3DLog::Severity::Error, "Unable to open configuration file ", fname);
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
    F3DLog::Print(F3DLog::Severity::Error, "Unable to parse the configuration file ", fname);
    return false;
  }

  // TODO

  return true;
}
