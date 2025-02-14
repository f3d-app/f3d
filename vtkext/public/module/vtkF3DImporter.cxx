#include "vtkF3DImporter.h"

#include <vtkInformationIntegerKey.h>

vtkInformationKeyMacro(vtkF3DImporter, ACTOR_IS_ARMATURE, Integer);

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)

//----------------------------------------------------------------------------
bool vtkF3DImporter::UpdateAtTimeValue(double vtkNotUsed(timeValue))
{
  return this->GetUpdateStatus() == vtkImporter::UpdateStatusEnum::SUCCESS;
}

#else

//----------------------------------------------------------------------------
bool vtkF3DImporter::UpdateAtTimeValue(double vtkNotUsed(timeValue))
{
  return true;
}

//----------------------------------------------------------------------------
void vtkF3DImporter::UpdateTimeStep(double timeValue)
{
  this->UpdateAtTimeValue(timeValue);
}

#endif

//----------------------------------------------------------------------------
void vtkF3DImporter::SetFailureStatus()
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  this->SetUpdateStatus(vtkImporter::UpdateStatusEnum::FAILURE);
#endif
}
