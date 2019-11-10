/**
 * @class   F3DOptions
 * @brief   The class that holds and manages options
 *
 */

#ifndef vtkF3DOptions_h
#define vtkF3DOptions_h

#include "Config.h"

#include "cxxopts.hpp"

#include <vtkObject.h>

class vtkF3DGenericImporter;
class vtkRenderer;
class vtkRenderWindow;

class F3DOptions
{
public:
  F3DOptions() = default;
  ~F3DOptions() = default;

  bool InitializeFromArgs(int argc, char** argv);
  bool InitializeFromFile(const std::string& fname);

  bool Normals = false;
  bool Axis = false;
  bool Grid = false;
  bool SSAO = false;
  bool FXAA = false;
  bool DepthPeeling = false;
  bool Verbose = false;
  bool Importer = false;
  std::vector<int> WindowSize;
  std::vector<double> BackgroundColor;
  std::string Input = "";
  std::string Reference = "";
  double RefThreshold = 0.1;
  std::string Output = "";
  std::string Scalars = "";
  int Component = -1;
  bool Cells = false;
  bool HideBar = false;
  bool HideProgress = false;
  std::vector<double> Range;
  bool FPS = false;
};

#endif
