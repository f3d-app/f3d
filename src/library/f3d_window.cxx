#include "f3d_window.h"

#include "f3d_options.h"

namespace f3d
{
//----------------------------------------------------------------------------
bool window::update()
{
  return false;
}

//----------------------------------------------------------------------------
bool window::render()
{
  return false;
}

//----------------------------------------------------------------------------
bool window::renderToFile(const std::string&, bool)
{
  return false;
}

//----------------------------------------------------------------------------
bool window::renderAndCompareWithFile(const std::string&, double, bool, const std::string&)
{
  return false;
}

//----------------------------------------------------------------------------
void window::SetOptions(const options* options)
{
  this->Options = options;
}

//----------------------------------------------------------------------------
void window::Initialize(bool, std::string) {}

//----------------------------------------------------------------------------
void window::InitializeRendererWithColoring(vtkF3DGenericImporter* importer) {}

};
