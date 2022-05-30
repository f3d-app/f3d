#include "window_impl_noRender.h"

#include <vtkNew.h>
#include <vtkRenderWindow.h>

namespace f3d::detail
{
class window_impl_noRender::internals
{
public:
  vtkNew<vtkRenderWindow> RenWin;
};

//----------------------------------------------------------------------------
window_impl_noRender::window_impl_noRender(const options& options)
  : window_impl(options)
  , Internals(new window_impl_noRender::internals)
{
  this->Internals->RenWin->SetOffScreenRendering(true);
}

//----------------------------------------------------------------------------
window_impl_noRender::~window_impl_noRender() = default;

//----------------------------------------------------------------------------
vtkRenderWindow* window_impl_noRender::GetRenderWindow()
{
  return this->Internals->RenWin;
}
};
