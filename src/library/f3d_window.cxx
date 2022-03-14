#include "f3d_window.h"

#include "f3d_options.h"

namespace f3d
{
//----------------------------------------------------------------------------
window::window(const options& options)
  : Options(options) {}

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
bool window::setIcon(const void*, size_t)
{
  return false;
}

//----------------------------------------------------------------------------
bool window::setWindowName(const std::string& windowName)
{
  return false;
}

//----------------------------------------------------------------------------
void window::Initialize(bool, std::string) {}

//----------------------------------------------------------------------------
void window::InitializeRendererWithColoring(vtkF3DGenericImporter*) {}

};
