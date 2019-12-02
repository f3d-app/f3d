#include "F3DOptions.h"

#include "F3DLog.h"

#include "vtkF3DGenericImporter.h"

#include <vtksys/SystemTools.hxx>

#include <fstream>
#include <regex>
#include <utility>
#include <vector>

class ConfigurationOptions
{
public:
  ConfigurationOptions(F3DOptions& parent, int argc, char** argv)
    : Parent(parent)
    , Argc(argc)
    , Argv(argv)
  {
  }

  bool Initialize()
  {
    this->InitializeFromConfigFile();
    return this->InitializeFromArgs(this->Parent);
  }

protected:
  bool InitializeFromArgs(F3DOptions&);
  bool InitializeFromConfigFile();

  bool ParseConfigurationFile(const std::string& configFilePath);

  template<class T>
  std::string GetOptionDefault(const std::string& option, T currValue)
  {
    auto it = this->ConfigOptions.find(option);
    if (it == this->ConfigOptions.end())
    {
      std::stringstream ss;
      ss << currValue;
      return ss.str();
    }
    return it->second;
  }

  std::string GetOptionDefault(const std::string& option, bool currValue)
  {
    auto it = this->ConfigOptions.find(option);
    if (it == this->ConfigOptions.end())
    {
      return currValue ? "true" : "false";
    }
    return it->second;
  }

  std::string CollapseName(const std::string& longName, const std::string& shortName)
  {
    std::stringstream ss;
    if (shortName != "")
    {
      ss << shortName << ",";
    }
    ss << longName;
    return ss.str();
  }

  template<class T>
  std::string GetOptionDefault(const std::string& option, const std::vector<T>& currValue)
  {
    auto it = this->ConfigOptions.find(option);
    if (it == this->ConfigOptions.end())
    {
      std::stringstream ss;
      for (size_t i = 0; i < currValue.size(); i++)
      {
        ss << currValue[i];
        if (i != currValue.size() - 1) ss << ",";
      }
      return ss.str();
    }
    return it->second;
  }

  void DeclareOption(cxxopts::OptionAdder& group, const std::string& longName,
    const std::string& shortName, const std::string& doc)
  {
    group(this->CollapseName(longName, shortName), doc);
  }

  template<class T>
  void DeclareOption(cxxopts::OptionAdder& group, const std::string& longName,
    const std::string& shortName, const std::string& doc, T& var, bool hasDefault = true,
    const std::string& argHelp = "")
  {
    auto val = cxxopts::value<T>(var);
    if (hasDefault)
    {
      val = val->default_value(this->GetOptionDefault(longName, var));
    }
    var = {};
    group(this->CollapseName(longName, shortName), doc, val, argHelp);
  }

  template<class T>
  void DeclareOption(cxxopts::OptionAdder& group, const std::string& longName,
    const std::string& shortName, const std::string& doc, T& var, const std::string& implicitValue,
    const std::string& argHelp = "")
  {
    auto val = cxxopts::value<T>(var)->implicit_value(implicitValue);
    var = {};
    group(this->CollapseName(longName, shortName), doc, val, argHelp);
  }

  static std::string GetUserSettingsDirectory();
  static std::string GetUserSettingsFilePath();

protected:
  F3DOptions& Parent;
  int Argc;
  char** Argv;

  using Dictionnary = std::map<std::string, std::string>;
  Dictionnary ConfigOptions;
};

//----------------------------------------------------------------------------
bool ConfigurationOptions::InitializeFromArgs(F3DOptions &options)
{
  try
  {
    cxxopts::Options cxxOptions(f3d::AppName, f3d::AppTitle);
    cxxOptions
      .positional_help("input_file")
      .show_positional_help();

    auto grp1 = cxxOptions.add_options();
    this->DeclareOption(grp1, "input", "", "Input file", options.Input, false, "<file>");
    this->DeclareOption(grp1, "output", "", "Render to file", options.Output, false, "<png file>");
    this->DeclareOption(grp1, "help", "h", "Print help");
    this->DeclareOption(grp1, "version", "", "Print version details");
    this->DeclareOption(grp1, "verbose", "v", "Enable verbose mode", options.Verbose);
    this->DeclareOption(grp1, "axis", "x", "Show axis", options.Axis);
    this->DeclareOption(grp1, "grid", "g", "Show grid", options.Grid);
    this->DeclareOption(grp1, "progress", "p", "Show progress bar", options.Progress);
    this->DeclareOption(grp1, "geometry-only", "m", "Do not read materials, cameras and lights from file", options.GeometryOnly);

    auto grp2 = cxxOptions.add_options("Material");
    this->DeclareOption(grp2, "edges", "e", "Show cell edges", options.Edges);
    this->DeclareOption(grp2, "point-size", "", "Point size", options.PointSize, true, "<size>");
    this->DeclareOption(grp2, "color", "", "Solid color", options.SolidColor, true, "<R,G,B>");
    this->DeclareOption(grp2, "opacity", "", "Opacity", options.Opacity, true, "<opacity>");
    this->DeclareOption(grp2, "roughness", "", "Roughness coefficient (0.0-1.0)", options.Roughness, true, "<roughness>");
    this->DeclareOption(grp2, "metallic", "", "Metallic coefficient (0.0-1.0)", options.Metallic, true, "<metallic>");

    auto grp3 = cxxOptions.add_options("Window");
    this->DeclareOption(grp3, "bg-color", "", "Background color", options.BackgroundColor, true, "<R,G,B>");
    this->DeclareOption(grp3, "resolution", "", "Window resolution", options.WindowSize, true, "<width,height>");
    this->DeclareOption(grp3, "timer", "t", "Display frame per second", options.FPS);

    auto grp4 = cxxOptions.add_options("Scientific visualization");
    this->DeclareOption(grp4, "scalars", "", "Color by scalars", options.Scalars, std::string("f3d_reserved"), "<array_name>");
    this->DeclareOption(grp4, "comp", "", "Specify the component used", options.Component, true, "<comp_index>");
    this->DeclareOption(grp4, "cells", "c", "The array is located on cells", options.Cells);
    this->DeclareOption(grp4, "range", "", "Custom range for the array", options.Range, false, "<min,max>");
    this->DeclareOption(grp4, "bar", "b", "Show scalar bar", options.Bar);

#if F3D_HAS_RAYTRACING
    auto grp5 = cxxOptions.add_options("Raytracing");
    this->DeclareOption(grp5, "raytracing", "r", "Enable raytracing", options.Raytracing);
    this->DeclareOption(grp5, "samples", "", "Number of samples per pixel", options.Samples, true, "<samples>");
    this->DeclareOption(grp5, "denoise", "s", "Denoise the image", options.Denoise);
#endif

    auto grp6 = cxxOptions.add_options("PostFX (OpenGL)");
    this->DeclareOption(grp6, "depth-peeling", "d", "Enable depth peeling", options.DepthPeeling);
    this->DeclareOption(grp6, "ssao", "u", "Enable Screen-Space Ambient Occlusion", options.SSAO);
    this->DeclareOption(grp6, "fxaa", "f", "Enable Fast Approximate Anti-Aliasing", options.FXAA);

    auto grp7 = cxxOptions.add_options("Testing");
    this->DeclareOption(grp7, "ref", "", "Reference", options.Reference, false, "<png file>");
    this->DeclareOption(grp7, "ref-threshold", "", "Testing threshold", options.RefThreshold, false, "<threshold>");

    cxxOptions.parse_positional({ "input", "positional" });

    if (this->Argc == 1)
    {
      F3DLog::Print(F3DLog::Severity::Info, cxxOptions.help());
      exit(EXIT_FAILURE);
    }
    int argc = this->Argc;
    auto result = cxxOptions.parse(argc, this->Argv);
    options.Input = result["input"].as<std::string>().c_str();

    if (result.count("help") > 0)
    {
      F3DLog::Print(F3DLog::Severity::Info, cxxOptions.help());
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
  }
  catch (const cxxopts::OptionException &e)
  {
    F3DLog::Print(F3DLog::Severity::Error, "Error parsing options: ", e.what());
    exit(EXIT_FAILURE);
  }
  return true;
}

//----------------------------------------------------------------------------
bool ConfigurationOptions::InitializeFromConfigFile()
{
  // Parse the args first to fetch the input file path
  F3DOptions opts;
  this->InitializeFromArgs(opts);

  const std::string& configFilePath = this->GetUserSettingsFilePath();
  std::ifstream file;
  file.open(configFilePath.c_str());

  if (!file.is_open())
  {
    return false;
  }

  Json::Value root;
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  std::string errs;
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  bool success = Json::parseFromStream(builder, file, &root, &errs);
  if (!success)
  {
    F3DLog::Print(F3DLog::Severity::Error, "Unable to parse the configuration file ", configFilePath);
    F3DLog::Print(F3DLog::Severity::Error, errs);
    return false;
  }

  for (auto const& id : root.getMemberNames())
  {
    std::regex re(id);
    std::smatch matches;
    if (std::regex_match(opts.Input, matches, re))
    {
      const Json::Value node = root[id];

      for (auto const& nl : node.getMemberNames())
      {
        const Json::Value v = node[nl];
        this->ConfigOptions[nl] = v.asString();
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------
std::string ConfigurationOptions::GetUserSettingsDirectory()
{
  std::string applicationName = "f3d";
#if defined(_WIN32)
  const char* appData = vtksys::SystemTools::GetEnv("APPDATA");
  if (!appData)
  {
    return std::string();
  }
  std::string separator("\\");
  std::string directoryPath(appData);
  if (directoryPath[directoryPath.size() - 1] != separator[0])
  {
    directoryPath.append(separator);
  }
  directoryPath += applicationName + separator;
#else
  std::string directoryPath;
  std::string separator("/");

  // Emulating QSettings behavior.
  const char* xdgConfigHome = getenv("XDG_CONFIG_HOME");
  if (xdgConfigHome && strlen(xdgConfigHome) > 0)
  {
    directoryPath = xdgConfigHome;
    if (directoryPath[directoryPath.size() - 1] != separator[0])
    {
      directoryPath += separator;
    }
  }
  else
  {
    const char* home = getenv("HOME");
    if (!home)
    {
      return std::string();
    }
    directoryPath = home;
    if (directoryPath[directoryPath.size() - 1] != separator[0])
    {
      directoryPath += separator;
    }
    directoryPath += ".config/";
  }
  directoryPath += applicationName + separator;
#endif
  return directoryPath;
}

//----------------------------------------------------------------------------
std::string ConfigurationOptions::GetUserSettingsFilePath()
{
  return ConfigurationOptions::GetUserSettingsDirectory() + "f3d.json";
}

//----------------------------------------------------------------------------
F3DOptionsParser::F3DOptionsParser(F3DOptions& options, int argc, char** argv)
  : ConfigOptions(new ConfigurationOptions(options, argc, argv))
{
  this->ConfigOptions->Initialize();
}

//----------------------------------------------------------------------------
F3DOptionsParser::~F3DOptionsParser()
{
  delete this->ConfigOptions;
}
