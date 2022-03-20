#include "f3d_window_impl.h"

#include "f3d_options.h"

namespace f3d
{
//----------------------------------------------------------------------------
window_impl::window_impl(const options& options)
  : Options(options)
{
}

//----------------------------------------------------------------------------
bool window_impl::update()
{
  return false;
}

//----------------------------------------------------------------------------
bool window_impl::render()
{
  return false;
}

//----------------------------------------------------------------------------
bool window_impl::renderToFile(const std::string&, bool)
{
  return false;
}

//----------------------------------------------------------------------------
bool window_impl::renderAndCompareWithFile(const std::string&, double, bool, const std::string&)
{
  return false;
}

//----------------------------------------------------------------------------
bool window_impl::setIcon(const void*, size_t)
{
  return false;
}

//----------------------------------------------------------------------------
bool window_impl::setWindowName(const std::string&)
{
  return false;
}

//----------------------------------------------------------------------------
void window_impl::Initialize(bool, std::string) {}

//----------------------------------------------------------------------------
void window_impl::InitializeRendererWithColoring(vtkF3DGenericImporter*) {}

};
