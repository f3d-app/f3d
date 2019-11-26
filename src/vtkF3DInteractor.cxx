#include "vtkF3DInteractor.h"

#include "F3DViewer.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>

#include <cassert>

vtkStandardNewMacro(vtkF3DInteractor);

//----------------------------------------------------------------------------
void vtkF3DInteractor::OnChar()
{
  vtkRenderWindowInteractor *rwi = this->GetInteractor();
  switch (rwi->GetKeyCode())
  {
    case 'q' :
    case 'Q' :
      rwi->ExitCallback();
      break;
    case 'r' :
    case 'R' :
      this->Viewer->GetRenderer()->ResetCamera();
      this->Viewer->Render();
      break;
    case 't':
    case 'T':
      this->Viewer->ShowTimer(!this->Viewer->IsTimerVisible());
      this->Viewer->Render();
      break;
    case 'x':
    case 'X':
      this->Viewer->ShowAxis(!this->Viewer->IsAxisVisible());
      this->Viewer->Render();
      break;
    case 'g':
    case 'G':
      this->Viewer->ShowGrid(!this->Viewer->IsGridVisible());
      this->Viewer->Render();
      break;
    case 'b':
    case 'B':
      this->Viewer->ShowScalarBar(!this->Viewer->IsScalarBarVisible());
      this->Viewer->Render();
      break;
    case 'e' :
    case 'E' :
      this->Viewer->ShowEdge(!this->Viewer->IsEdgeVisible());
      this->Viewer->Render();
    break;
    default:
      break;
  }
}
