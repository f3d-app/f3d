#include "window_impl_noRender.h"

#include "vtkF3DNoRenderWindow.h"

#include <vtkNew.h>

namespace f3d::detail
{
class window_impl_noRender::internals
{
public:
  vtkNew<vtkF3DNoRenderWindow> RenWin;
};

//----------------------------------------------------------------------------
window_impl_noRender::window_impl_noRender(const options& options)
  : window_impl(options)
  , Internals(new window_impl_noRender::internals)
{
}

//----------------------------------------------------------------------------
window_impl_noRender::~window_impl_noRender() = default;

//----------------------------------------------------------------------------
vtkRenderWindow* window_impl_noRender::GetRenderWindow()
{
  return this->Internals->RenWin;
}
};
