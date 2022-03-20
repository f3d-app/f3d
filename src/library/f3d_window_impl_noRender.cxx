#include "f3d_window_impl_noRender.h"

#include <vtkNew.h>
#include <vtkRenderWindow.h>

namespace f3d
{
class window_impl_noRender::F3DInternals
{
public:
  vtkNew<vtkRenderWindow> RenWin;
};

//----------------------------------------------------------------------------
window_impl_noRender::window_impl_noRender(const options& options)
  : window_impl(options)
  , Internals(new window_impl_noRender::F3DInternals)
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
