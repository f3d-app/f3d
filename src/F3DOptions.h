/**
 * @class   F3DOptions
 * @brief   The class that holds and manages options
 *
 */

#ifndef vtkF3DOptions_h
#define vtkF3DOptions_h

#include "Config.h"

#include <memory>
#include <vector>

class ConfigurationOptions;

struct F3DOptions
{
  bool Axis = false;
  bool Bar = false;
  bool Cells = false;
  bool Denoise = false;
  bool DepthPeeling = false;
  bool DryRun = false;
  bool Edges = false;
  bool FPS = false;
  bool Filename = false;
  bool FXAA = false;
  bool GeometryOnly = false;
  bool Grid = false;
  bool Progress = false;
  bool Raytracing = false;
  bool SSAO = false;
  bool Verbose = false;
  bool NoRender = false;
  bool PointSprites = false;
  bool FullScreen = false;
  bool ToneMapping = false;
  double CameraViewAngle;
  double Metallic = 0.0;
  double Opacity = 1.0;
  double PointSize = 10.0;
  double RefThreshold = 0.1;
  double Roughness = 0.3;
  int Component = -1;
  int Samples = 5;
  std::string Output = "";
  std::string Reference = "";
  std::string Scalars = f3d::F3DReservedString;
  std::vector<double> BackgroundColor = { 0.2, 0.2, 0.2 };
  std::vector<double> CameraPosition;
  std::vector<double> CameraFocalPoint;
  std::vector<double> CameraViewUp;
  std::vector<double> LookupPoints;
  std::vector<double> Range;
  std::vector<double> SolidColor = { 1., 1., 1. };
  std::vector<int> WindowSize = { 1000, 600 };
  std::string HDRIFile;
};

class F3DOptionsParser
{
public:
  void Initialize(int argc, char** argv);

  /**
   * Parse the command line and return the options passed
   * The provided inputs arguments will also be filled by the
   * positional inputs or inputs arguments from command line.
   * Returns the resulting options.
   */
  F3DOptions GetOptionsFromCommandLine(std::vector<std::string>& inputs);

  /**
   * Parse the command line and return the options passed.
   * Convenience method that does not allows to recover the inputs.
   */
  F3DOptions GetOptionsFromCommandLine();

  /**
   * Parse the config file in different potential location
   * using the provided filepath to match the regexp in
   * the config files. Then parse the command line for any supplemental.
   * Returns the resulting options.
   */
  F3DOptions GetOptionsFromConfigFile(const std::string& filePath);

  /**
   * Check the validity of a provided option
   * and print to the log if they are not.
   * return true if all options are compatible, false otherwise.
   */
  static bool CheckValidity(const F3DOptions& options, const std::string& filePath);

  F3DOptionsParser();
  ~F3DOptionsParser();

private:
  F3DOptionsParser(F3DOptionsParser const&) = delete;
  void operator=(F3DOptionsParser const&) = delete;

  std::unique_ptr<ConfigurationOptions> ConfigOptions;
};

#endif
