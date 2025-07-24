#include "vtkF3DGenericImporter.h"

#include "F3DLog.h"
#include "vtkF3DPostProcessFilter.h"

#include <vtkActor.h>
#include <vtkEventForwarderCommand.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVersion.h>

#include <cassert>
#include <sstream>

struct vtkF3DGenericImporter::Internals
{
  vtkSmartPointer<vtkAlgorithm> Reader = nullptr;
  vtkNew<vtkF3DPostProcessFilter> PostPro;
  vtkNew<vtkActor> GeometryActor;
  vtkNew<vtkPolyDataMapper> PolyDataMapper;
  std::string OutputDescription;

  vtkPolyData* ImportedPoints = nullptr;
  vtkImageData* ImportedImage = nullptr;

  bool HasAnimation = false;
  bool AnimationEnabled = false;
  std::array<double, 2> TimeRange;
};

vtkStandardNewMacro(vtkF3DGenericImporter);

//----------------------------------------------------------------------------
vtkF3DGenericImporter::vtkF3DGenericImporter()
  : Pimpl(new Internals())
{
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::UpdateTemporalInformation()
{
  assert(this->Pimpl->Reader);
  this->Pimpl->HasAnimation = false;
  this->Pimpl->Reader->UpdateInformation();
  vtkInformation* readerInfo = this->Pimpl->Reader->GetOutputInformation(0);
  if (readerInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_RANGE()))
  {
    const double* readerTimeRange = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE());
    this->Pimpl->TimeRange[0] = readerTimeRange[0];
    this->Pimpl->TimeRange[1] = readerTimeRange[1];
    this->Pimpl->HasAnimation = true;
  }
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DGenericImporter::GetNumberOfAnimations()
{
  return this->Pimpl->HasAnimation ? 1 : 0;
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetAnimationName([[maybe_unused]] vtkIdType animationIndex)
{
  assert(animationIndex == 0);
  return this->Pimpl->HasAnimation ? "default" : "";
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::EnableAnimation([[maybe_unused]] vtkIdType animationIndex)
{
  assert(animationIndex == 0);
  if (this->Pimpl->HasAnimation)
  {
    this->Pimpl->AnimationEnabled = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::DisableAnimation([[maybe_unused]] vtkIdType animationIndex)
{
  assert(animationIndex == 0);
  if (this->Pimpl->HasAnimation)
  {
    this->Pimpl->AnimationEnabled = false;
  }
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::IsAnimationEnabled([[maybe_unused]] vtkIdType animationIndex)
{
  assert(animationIndex == 0);
  return this->Pimpl->AnimationEnabled;
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::GetTemporalInformation([[maybe_unused]] vtkIdType animationIndex,
  double vtkNotUsed(frameRate), int& vtkNotUsed(nbTimeSteps), double timeRange[2],
  vtkDoubleArray* vtkNotUsed(timeSteps))
{
  assert(animationIndex == 0);
  // F3D do not care about timesteps
  if (this->Pimpl->HasAnimation)
  {
    timeRange[0] = this->Pimpl->TimeRange[0];
    timeRange[1] = this->Pimpl->TimeRange[1];
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportActors(vtkRenderer* ren)
{
  assert(this->Pimpl->Reader);

  // Read file and forward progress
  vtkNew<vtkEventForwarderCommand> progressForwarder;
  progressForwarder->SetTarget(this);
  this->Pimpl->Reader->AddObserver(vtkCommand::ProgressEvent, progressForwarder);
  bool status = this->Pimpl->PostPro->GetExecutive()->Update();
  if (!status || !this->Pimpl->Reader->GetOutputDataObject(0))
  {
    this->SetFailureStatus();
    return;
  }

  // Cast to dataset types
  this->Pimpl->ImportedPoints = vtkPolyData::SafeDownCast(this->Pimpl->PostPro->GetOutput(1));
  vtkImageData* image = vtkImageData::SafeDownCast(this->Pimpl->PostPro->GetOutput(2));
  this->Pimpl->ImportedImage = image->GetNumberOfCells() > 0 ? image : nullptr;

  // Recover output description from the reader
  this->Pimpl->OutputDescription =
    vtkF3DGenericImporter::GetDataObjectDescription(this->Pimpl->Reader->GetOutputDataObject(0));

  // Add filter outputs to mapper inputs
  this->Pimpl->PolyDataMapper->SetInputConnection(this->Pimpl->PostPro->GetOutputPort(0));
  this->Pimpl->PolyDataMapper->ScalarVisibilityOff();

  // Set geometry actor default properties
  // Rely on vtkProperty default for other properties
  this->Pimpl->GeometryActor->GetProperty()->SetPointSize(10.0);
  this->Pimpl->GeometryActor->GetProperty()->SetLineWidth(1.0);
  this->Pimpl->GeometryActor->GetProperty()->SetRoughness(0.3);
  this->Pimpl->GeometryActor->GetProperty()->SetBaseIOR(1.5);
  this->Pimpl->GeometryActor->GetProperty()->SetInterpolationToPBR();

  // add mappers
  // TODO implement proper composite support
  this->Pimpl->GeometryActor->SetMapper(this->Pimpl->PolyDataMapper);

  // add props
  ren->AddActor(this->Pimpl->GeometryActor);
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  this->ActorCollection->AddItem(this->Pimpl->GeometryActor);
#endif

  // Set visibilities
  this->Pimpl->GeometryActor->VisibilityOn();

  this->UpdateTemporalInformation();
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::SetInternalReader(vtkAlgorithm* reader)
{
  if (reader)
  {
    this->Pimpl->Reader = reader;
    this->Pimpl->PostPro->SetInputConnection(this->Pimpl->Reader->GetOutputPort());
  }
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetOutputsDescription()
{
  return this->Pimpl->OutputDescription;
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetMultiBlockDescription(
  vtkMultiBlockDataSet* mb, vtkIndent indent)
{
  std::stringstream ss;
  for (unsigned int i = 0; i < mb->GetNumberOfBlocks(); i++)
  {
    const char* blockName = mb->GetMetaData(i)->Get(vtkCompositeDataSet::NAME());
    ss << indent << "Block: " << (blockName ? std::string(blockName) : std::to_string(i)) << "\n";
    vtkDataObject* object = mb->GetBlock(i);
    vtkMultiBlockDataSet* mbChild = vtkMultiBlockDataSet::SafeDownCast(object);
    vtkDataSet* ds = vtkDataSet::SafeDownCast(object);
    if (mbChild)
    {
      ss << vtkF3DGenericImporter::GetMultiBlockDescription(mbChild, indent.GetNextIndent());
    }
    else if (ds)
    {
      ss << vtkImporter::GetDataSetDescription(ds, indent.GetNextIndent());
    }
  }
  return ss.str();
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetPartitionedDataSetCollectionDescription(
  vtkPartitionedDataSetCollection* pdc, vtkIndent indent)
{
  std::stringstream ss;
  for (unsigned int i = 0; i < pdc->GetNumberOfPartitionedDataSets(); i++)
  {
    const char* pdsName = pdc->GetMetaData(i)->Get(vtkCompositeDataSet::NAME());
    ss << indent << "PartitionedDataSet: " << (pdsName ? std::string(pdsName) : std::to_string(i))
       << "\n";
    vtkPartitionedDataSet* pds = pdc->GetPartitionedDataSet(i);
    for (unsigned int j = 0; j < pds->GetNumberOfPartitions(); j++)
    {
      vtkIndent nextIndent = indent.GetNextIndent();
      const char* pName = pds->GetMetaData(j)->Get(vtkCompositeDataSet::NAME());
      ss << nextIndent << "Partition: " << (pName ? std::string(pName) : std::to_string(j)) << "\n";
      vtkDataSet* ds = pds->GetPartition(j);
      if (ds)
      {
        ss << vtkImporter::GetDataSetDescription(ds, nextIndent.GetNextIndent());
      }
    }
  }
  return ss.str();
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetDataObjectDescription(vtkDataObject* object)
{
  vtkMultiBlockDataSet* mb = vtkMultiBlockDataSet::SafeDownCast(object);
  vtkPartitionedDataSetCollection* pdc = vtkPartitionedDataSetCollection::SafeDownCast(object);
  vtkDataSet* ds = vtkDataSet::SafeDownCast(object);
  if (mb)
  {
    return vtkF3DGenericImporter::GetMultiBlockDescription(mb, vtkIndent(0));
  }
  if (pdc)
  {
    return vtkF3DGenericImporter::GetPartitionedDataSetCollectionDescription(pdc, vtkIndent(0));
  }
  else if (ds)
  {
    return vtkImporter::GetDataSetDescription(ds, vtkIndent(0));
  }
  return "";
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::UpdateAtTimeValue(double timeValue)
{

  if (!this->Pimpl->AnimationEnabled)
  {
    // Animation is not enabled, nothing to do
    return true;
  }

  assert(this->Pimpl->Reader);
  if (!this->Pimpl->PostPro->UpdateTimeStep(timeValue) ||
    !this->Pimpl->Reader->GetOutputDataObject(0))
  {
    F3DLog::Print(F3DLog::Severity::Warning, "A reader failed to update at a timeValue");
    return false;
  }

  this->UpdateOutputDescriptions();
  return true;
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::UpdateOutputDescriptions()
{
  assert(this->Pimpl->Reader);
  // Recover output description
  vtkDataObject* readerOutput = this->Pimpl->Reader->GetOutputDataObject(0);
  this->Pimpl->OutputDescription = vtkF3DGenericImporter::GetDataObjectDescription(readerOutput);
}

//----------------------------------------------------------------------------
vtkPolyData* vtkF3DGenericImporter::GetImportedPoints()
{
  return this->Pimpl->ImportedPoints;
}

//----------------------------------------------------------------------------
vtkImageData* vtkF3DGenericImporter::GetImportedImage()
{
  return this->Pimpl->ImportedImage;
}
