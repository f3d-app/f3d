#include "vtkF3DInteractor.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkF3DInteractor);

//----------------------------------------------------------------------------
void vtkF3DInteractor::OnChar()
{
}

//----------------------------------------------------------------------------
void vtkF3DInteractor::OnKeyPress()
{
  vtkRenderWindowInteractor *rwi = this->GetInteractor();
  char keyCode = rwi->GetKeyCode();
  std::string keyname = rwi->GetKeySym();
  int shift = rwi->GetShiftKey();
  switch (keyCode)
  {
    case 'h':
      cout << "HELP!" << endl;
      break;
    case 'q':
      rwi->ExitCallback();
    default:
      break;
  }
}
