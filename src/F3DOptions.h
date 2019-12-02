/**
 * @class   F3DOptions
 * @brief   The class that holds and manages options
 *
 */

#ifndef vtkF3DOptions_h
#define vtkF3DOptions_h

#include "Config.h"

#include "cxxopts.hpp"

#include <vtk_jsoncpp.h>

#include <vtkObject.h>

class vtkF3DGenericImporter;
class vtkRenderer;
class vtkRenderWindow;

class ConfigurationOptions;

struct F3DOptions
{
  bool Axis = false;
  bool Bar = false;
  bool Cells = false;
  bool Denoise = false;
  bool DepthPeeling = false;
  bool Edges = false;
  bool FPS = false;
  bool FXAA = false;
  bool GeometryOnly = false;
  bool Grid = false;
  bool Progress = false;
  bool Raytracing = false;
  bool SSAO = false;
  bool Verbose = false;
  double Metallic = 0.0;
  double Opacity = 1.0;
  double PointSize = 10.0;
  double RefThreshold = 0.1;
  double Roughness = 0.3;
  int Component = -1;
  int Samples = 5;
  std::string Input = "";
  std::string Output = "";
  std::string Reference = "";
  std::string Scalars = "";
  std::vector<double> BackgroundColor = { 0.2, 0.2, 0.2 };
  std::vector<double> Range;
  std::vector<double> SolidColor = { 1., 1., 1. };
  std::vector<int> WindowSize = { 1000, 600 };
};

class F3DOptionsParser
{
public:
  F3DOptionsParser(F3DOptions& options, int argc, char** argv);
  ~F3DOptionsParser();

protected:
  ConfigurationOptions* ConfigOptions = nullptr;
};

#endif
