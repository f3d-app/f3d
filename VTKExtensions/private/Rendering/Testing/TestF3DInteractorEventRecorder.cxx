#include <vtkNew.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTestUtilities.h>

#include "vtkF3DInteractorEventRecorder.h"

int TestF3DInteractorEventRecorder(int argc, char* argv[])
{
  vtkNew<vtkF3DInteractorEventRecorder> record;
  vtkNew<vtkRenderWindowInteractor> inter;

  // Test Custom SetInteractorLogic
  record->SetInteractor(nullptr);
  record->SetInteractor(inter);
  record->SetEnabled(1);
  record->SetInteractor(nullptr);

  return record->GetEnabled() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
