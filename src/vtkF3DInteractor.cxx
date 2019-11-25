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
    case 'w' :
    case 'W' :
      this->Viewer->ShowEdge(!this->Viewer->IsEdgeVisible());
      this->Viewer->Render();
    break;
    default:
      break;
  }
}
