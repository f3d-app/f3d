#include "vtkF3DInteractor.h"

#include "F3DViewer.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>

#include <cassert>

vtkStandardNewMacro(vtkF3DInteractor);

//----------------------------------------------------------------------------
void vtkF3DInteractor::OnChar()
{
  this->Superclass::OnChar();

  assert(this->Viewer);

  vtkRenderWindowInteractor *rwi = this->GetInteractor();
  char keyCode = rwi->GetKeyCode();
  std::string keyname = rwi->GetKeySym();
  int shift = rwi->GetShiftKey();
  switch (keyCode)
  {
    case 'x':
      this->Viewer->ShowAxis(!this->Viewer->IsAxisVisible());
      this->Viewer->Render();
      break;
    case 'g':
      this->Viewer->ShowGrid(!this->Viewer->IsGridVisible());
      this->Viewer->Render();
      break;
    case 'b':
      this->Viewer->ShowScalarBar(!this->Viewer->IsScalarBarVisible());
      this->Viewer->Render();
      break;
    default:
      break;
  }
}
