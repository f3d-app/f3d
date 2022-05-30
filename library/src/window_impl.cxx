#include "window_impl.h"

#include "options.h"

namespace f3d::detail
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
image window_impl::renderToImage(bool)
{
  return image();
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
