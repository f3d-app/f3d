#include "vtkF3DImporter.h"

#include <vtkInformationIntegerKey.h>

vtkInformationKeyMacro(vtkF3DImporter, ACTOR_IS_ARMATURE, Integer);

//----------------------------------------------------------------------------
bool vtkF3DImporter::UpdateAtTimeValue(double vtkNotUsed(timeValue))
{
  return this->GetUpdateStatus() == vtkImporter::UpdateStatusEnum::SUCCESS;
}

#if VTK_VERSION_NUMBER < VTK_VERSION_CHECK(9, 5, 20251210)
//----------------------------------------------------------------------------
bool vtkF3DImporter::GetTemporalInformation(
  vtkIdType animationIndex, double timeRange[2], int& nbTimeSteps, vtkDoubleArray* timeSteps)
{
  return this->vtkImporter::GetTemporalInformation(
    animationIndex, 0, nbTimeSteps, timeRange, timeSteps);
}
#endif

//----------------------------------------------------------------------------
void vtkF3DImporter::SetFailureStatus()
{
  this->SetUpdateStatus(vtkImporter::UpdateStatusEnum::FAILURE);
}
