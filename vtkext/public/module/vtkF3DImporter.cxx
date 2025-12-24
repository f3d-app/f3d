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
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  this->SetUpdateStatus(vtkImporter::UpdateStatusEnum::FAILURE);
#endif
}
