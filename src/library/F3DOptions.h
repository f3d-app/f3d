/**
 * @class   F3DOptions
 * @brief   The class that holds and manages options
 *
 */

#ifndef F3DOptions_h
#define F3DOptions_h

#include "F3DConfig.h"

#include <memory>
#include <vector>

class ConfigurationOptions;
namespace f3d{ class options;}

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
  bool MetaData = false;
  bool FXAA = false;
  bool GeometryOnly = false;
  bool Grid = false;
  bool Progress = false;
  bool Raytracing = false;
  bool SSAO = false;
  bool Verbose = false;
  bool NoRender = false;
  bool Quiet = false;
  bool PointSprites = false;
  bool FullScreen = false;
  bool ToneMapping = false;
  bool Volume = false;
  bool InverseOpacityFunction = false;
  bool NoBackground = false;
  bool BlurBackground = false;
  bool Trackball = false;
  double CameraViewAngle;
  double CameraAzimuthAngle = 0.0;
  double CameraElevationAngle = 0.0;
  double Metallic = 0.0;
  double Opacity = 1.0;
  double PointSize = 10.0;
  double LineWidth = 1.0;
  double RefThreshold = 0.1;
  double Roughness = 0.3;
  int Component = -1;
  int Samples = 5;
  std::string Up = "+Y";
  int AnimationIndex = 0;
  int CameraIndex = -1;
  std::string UserConfigFile = "";
  std::string Output = "";
  std::string Reference = "";
  std::string Scalars = f3d::F3DReservedString;
  std::string InteractionTestRecordFile = "";
  std::string InteractionTestPlayFile = "";
  std::vector<double> BackgroundColor = { 0.2, 0.2, 0.2 };
  std::vector<double> CameraPosition;
  std::vector<double> CameraFocalPoint;
  std::vector<double> CameraViewUp;
  std::vector<double> Colormap = { 0.0, 0.0, 0.0, 0.0, 0.4, 0.9, 0.0, 0.0, 0.8, 0.9, 0.9, 0.0,
    1.0, 1.0, 1.0, 1.0 };
  std::vector<double> Range;
  std::vector<double> SolidColor = { 1., 1., 1. };
  std::vector<int> Resolution = { 1000, 600 };
  std::string HDRIFile;
  std::string BaseColorTex;
  std::string ORMTex;
  std::string EmissiveTex;
  std::vector<double> EmissiveFactor = { 1., 1., 1. };
  std::string NormalTex;
  double NormalScale = 1.0;
  std::string FontFile = "";
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
  F3DOptions GetOptionsFromCommandLine(std::vector<std::string>& inputs);

  /**
   * Use the config file dictionnary using the provided filepath
   * to match the regexp from the config files.
   * Then parse the command line for any supplemental.
   * Returns the resulting options.
   */
  F3DOptions GetOptionsFromConfigFile(const std::string& filePath);

  F3DOptionsParser();
  ~F3DOptionsParser();

  static void ConvertToNewAPI(const F3DOptions& oldOptions, f3d::options* newOptions);

private:
  F3DOptionsParser(F3DOptionsParser const&) = delete;
  void operator=(F3DOptionsParser const&) = delete;

  std::unique_ptr<ConfigurationOptions> ConfigOptions;
};

#endif
