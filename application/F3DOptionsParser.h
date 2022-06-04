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
  std::vector<int> Resolution{ 1000, 600 };
};

class F3DOptionsParser
{
public:
  void Initialize(int argc, char** argv);

  /**
   * Find and parse a config file, if any, into the config file dictionnary.
   * If a non-empty userConfigFile is provided, it will be considered instead
   * of standard settings config file
   */
  void InitializeDictionaryFromConfigFile(const std::string& userConfigFile);

  /**
   * Parse the command line and return the options passed
   * The provided inputs arguments will also be filled by the
   * positional inputs or inputs arguments from command line.
   * This will also reset the FilePathForConfigFile in order to
   * ignore config file options.
   * Returns the resulting options.
   */
  void GetOptionsFromCommandLine(
    F3DAppOptions& addOptions, f3d::options& options, std::vector<std::string>& inputs);

  /**
   * Use the config file dictionnary using the provided filepath
   * to match the regexp from the config files.
   * Then parse the command line for any supplemental.
   * Returns the resulting options.
   */
  void GetOptionsFromConfigFile(const std::string& filePath, f3d::options& options);

  F3DOptionsParser();
  ~F3DOptionsParser();

private:
  F3DOptionsParser(F3DOptionsParser const&) = delete;
  void operator=(F3DOptionsParser const&) = delete;

  std::unique_ptr<ConfigurationOptions> ConfigOptions;
};

#endif
