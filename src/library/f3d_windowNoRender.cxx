#include "f3d_windowNoRender.h"

#include <vtkNew.h>
#include <vtkRenderWindow.h>

namespace f3d
{
class windowNoRender::F3DInternals
{
public:
  vtkNew<vtkRenderWindow> RenWin;
};

//----------------------------------------------------------------------------
windowNoRender::windowNoRender()
  : Internals(new windowNoRender::F3DInternals)
{
}

//----------------------------------------------------------------------------
windowNoRender::~windowNoRender() = default;

//----------------------------------------------------------------------------
vtkRenderWindow* windowNoRender::GetRenderWindow()
{
  return this->Internals->RenWin;
}
};
