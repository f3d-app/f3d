/**
 * @class   F3DOptions
 * @brief   The main viewer class
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
  std::vector<int> WindowSize;
  std::string Input = "";
};

#endif
