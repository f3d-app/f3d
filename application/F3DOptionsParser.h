/**
 * @class   F3DOptionsParser
 * @brief   A class to parse application and library options
 *
 */

#ifndef F3DOptionsParser_h
#define F3DOptionsParser_h

#include <memory>
#include <string>
#include <vector>

class ConfigurationOptions;
namespace f3d
{
class options;
}

struct F3DAppOptions
{
  std::string UserConfigFile = "";
  bool DryRun = false;
  std::string Output = "";
  std::string Reference = "";
  std::string InteractionTestRecordFile = "";
  std::string InteractionTestPlayFile = "";
  bool NoBackground = false;
  bool NoRender = false;
  double RefThreshold = 50;
  int MaxSize = -1;

  std::vector<int> Resolution{ 1000, 600 };
  std::vector<int> Position{ 0 };
  bool Quiet = false;
  bool Verbose = false;
  double CameraAzimuthAngle = 0.0;
  double CameraElevationAngle = 0.0;
  std::vector<double> CameraFocalPoint = { 0 };
  std::vector<double> CameraPosition = { 0 };
  std::vector<double> CameraViewUp = { 0 };
  double CameraViewAngle = 0.0;
  std::vector<std::string> Plugins;
};

class F3DOptionsParser
{
public:
  void Initialize(int argc, char** argv);

  /**
   * Find and parse a config file, if any, into the config file dictionary.
   * If a non-empty configNameis provided, it will be considered instead
   * of standard settings config file.
   * supported config are:
   *  - relative/absolute path to a config file
   *  - name of file to look for in standard locations
   *  - name of file without extension to look for in standard locations, .json will be added.
   */
  void InitializeDictionaryFromConfigFile(const std::string& config);

  /**
   * Parse the command line and return the options passed
   * The provided inputs arguments will also be filled by the
   * positional inputs or inputs arguments from command line.
   * This will consider the global config file only
   * Returns the resulting options.
   */
  void GetOptions(
    F3DAppOptions& appOptions, f3d::options& options, std::vector<std::string>& inputs);

  /**
   * Use the config file dictionary using the provided filepath
   * to match the regexp from the config files.
   * This will not parse the command line.
   * Returns the resulting options.
   */
  void UpdateOptions(const std::string& filePath, F3DAppOptions& appOptions, f3d::options& options,
    bool parseCommandLine);

  /**
   * Load the plugins specified in the app options.
   */
  void LoadPlugins(const F3DAppOptions& appOptions) const;

  F3DOptionsParser();
  ~F3DOptionsParser();

private:
  F3DOptionsParser(F3DOptionsParser const&) = delete;
  void operator=(F3DOptionsParser const&) = delete;

  std::unique_ptr<ConfigurationOptions> ConfigOptions;
};

#endif
