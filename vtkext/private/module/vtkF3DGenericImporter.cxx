#include "vtkF3DGenericImporter.h"

#include "F3DLog.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCellData.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSetGet.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVolume.h>

#include <sstream>

struct ReaderPipeline
{
  ReaderPipeline()
  {
    this->GeometryActor->GetProperty()->SetInterpolationToPBR();
    this->VolumeMapper->SetRequestedRenderModeToGPU();
    this->PolyDataMapper->InterpolateScalarsBeforeMappingOn();
  }

  std::string Name;
  bool Imported = false;
  vtkSmartPointer<vtkAlgorithm> Reader;
  vtkNew<vtkF3DPostProcessFilter> PostPro;
  std::string OutputDescription;

  vtkNew<vtkActor> GeometryActor;
  vtkNew<vtkActor> PointSpritesActor;
  vtkNew<vtkVolume> VolumeProp;
  vtkNew<vtkPolyDataMapper> PolyDataMapper;
  vtkNew<vtkPointGaussianMapper> PointGaussianMapper;
  vtkNew<vtkSmartVolumeMapper> VolumeMapper;

  vtkDataSet* Output = nullptr;
  vtkDataSetAttributes* PointDataForColoring = nullptr;
  vtkDataSetAttributes* CellDataForColoring = nullptr;
};

struct vtkF3DGenericImporter::Internals
{
  std::vector<ReaderPipeline> Readers;

  std::vector<ColoringInfo> PointDataArrayVectorForColoring;
  std::vector<ColoringInfo> CellDataArrayVectorForColoring;
  vtkBoundingBox GeometryBoundingBox;

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
  this->Pimpl->HasAnimation = false;
  this->Pimpl->TimeRange[0] = std::numeric_limits<double>::infinity();
  this->Pimpl->TimeRange[1] = -std::numeric_limits<double>::infinity();

  // Update each reader
  for (ReaderPipeline& pipe : this->Pimpl->Readers)
  {
    pipe.Reader->UpdateInformation();
    vtkInformation* readerInfo = pipe.Reader->GetOutputInformation(0);
    if (readerInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_RANGE()))
    {
      // Accumulate time ranges
      double* readerTimeRange = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE());
      this->Pimpl->TimeRange[0] = std::min(this->Pimpl->TimeRange[0], readerTimeRange[0]);
      this->Pimpl->TimeRange[1] = std::max(this->Pimpl->TimeRange[1], readerTimeRange[1]);
      this->Pimpl->HasAnimation = true;
    }
  }
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DGenericImporter::GetNumberOfAnimations()
{
  return this->Pimpl->HasAnimation ? 1 : 0;
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetAnimationName(vtkIdType animationIndex)
{
  return animationIndex < this->GetNumberOfAnimations() ? "default" : "";
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::EnableAnimation(vtkIdType animationIndex)
{
  if (animationIndex < this->GetNumberOfAnimations())
  {
    this->Pimpl->AnimationEnabled = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::DisableAnimation(vtkIdType animationIndex)
{
  if (animationIndex < this->GetNumberOfAnimations())
  {
    this->Pimpl->AnimationEnabled = false;
  }
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::IsAnimationEnabled(vtkIdType animationIndex)
{
  return animationIndex < this->GetNumberOfAnimations() ? this->Pimpl->AnimationEnabled : false;
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::GetTemporalInformation(vtkIdType animationIndex,
  double vtkNotUsed(frameRate), int& vtkNotUsed(nbTimeSteps), double timeRange[2],
  vtkDoubleArray* vtkNotUsed(timeSteps))
{
  // F3D do not care about timesteps
  if (animationIndex < this->GetNumberOfAnimations())
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
  this->Pimpl->GeometryBoundingBox.Reset();
  bool hasGeometry = false;

  // Update each reader
  for (size_t readerIndex = 0; readerIndex < this->Pimpl->Readers.size(); readerIndex++)
  {
    ReaderPipeline& pipe = this->Pimpl->Readers[readerIndex];
    if (pipe.Imported)
    {
      continue;
    }

    {
      // Forward progress event
      vtkNew<vtkCallbackCommand> progressCallback;
      double progressRatio = static_cast<double>(readerIndex + 1) / this->Pimpl->Readers.size();
      std::pair<vtkF3DGenericImporter*, double> progressPair = { this, progressRatio };
      progressCallback->SetClientData(&progressPair);
      progressCallback->SetCallback(
        [](vtkObject*, unsigned long, void* clientData, void* callData)
        {
          auto localPair = static_cast<std::pair<vtkF3DGenericImporter*, double>*>(clientData);
          vtkF3DGenericImporter* self = localPair->first;
          double progress = *static_cast<double*>(callData) * localPair->second;
          self->InvokeEvent(vtkCommand::ProgressEvent, &progress);
        });
      pipe.Reader->AddObserver(vtkCommand::ProgressEvent, progressCallback);
      bool status = pipe.PostPro->GetExecutive()->Update();
      pipe.Reader->RemoveObservers(vtkCommand::ProgressEvent);

      if (!status || !pipe.Reader->GetOutputDataObject(0))
      {
        F3DLog::Print(F3DLog::Severity::Warning, "A reader failed to update");
        pipe.Output = nullptr;
        continue;
      }
    }

    // Cast to dataset types
    vtkPolyData* surface = vtkPolyData::SafeDownCast(pipe.PostPro->GetOutput());
    vtkImageData* image = vtkImageData::SafeDownCast(pipe.PostPro->GetOutput(2));
    vtkDataSet* dataSet = vtkImageData::SafeDownCast(pipe.PostPro->GetInput())
      ? vtkDataSet::SafeDownCast(image)
      : vtkDataSet::SafeDownCast(surface);
    pipe.Output = dataSet;

    // Recover output description from the reader
    pipe.OutputDescription = vtkF3DGenericImporter::GetDataObjectDescription(pipe.Reader->GetOutputDataObject(0));

    // Recover data for coloring
    pipe.PointDataForColoring = vtkDataSetAttributes::SafeDownCast(dataSet->GetPointData());
    pipe.CellDataForColoring = vtkDataSetAttributes::SafeDownCast(dataSet->GetCellData());

    // Increase bounding box size if needed
    double bounds[6];
    surface->GetBounds(bounds);
    this->Pimpl->GeometryBoundingBox.AddBounds(bounds);

    // Add filter outputs to mapper inputs
    pipe.PolyDataMapper->SetInputConnection(pipe.PostPro->GetOutputPort(0));
    pipe.PointGaussianMapper->SetInputConnection(pipe.PostPro->GetOutputPort(1));
    pipe.VolumeMapper->SetInputConnection(pipe.PostPro->GetOutputPort(2));

    // add mappers
    pipe.VolumeProp->SetMapper(pipe.VolumeMapper);
    pipe.GeometryActor->SetMapper(pipe.PolyDataMapper);
    pipe.PointSpritesActor->SetMapper(pipe.PointGaussianMapper);

    // add props
    ren->AddActor(pipe.GeometryActor);
    ren->AddActor(pipe.PointSpritesActor);
    ren->AddVolume(pipe.VolumeProp);

    // Set visibilities
    pipe.GeometryActor->VisibilityOff();
    pipe.PointSpritesActor->VisibilityOff();
    pipe.VolumeProp->VisibilityOff();

    pipe.Imported = true;
    hasGeometry = true;
  }

  this->UpdateTemporalInformation();
  this->UpdateColoringVectors(false);
  this->UpdateColoringVectors(true);
  if (!hasGeometry)
  {
    this->SetFailureStatus();
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::PrintSelf(std::ostream& os, vtkIndent indent)
{
  vtkImporter::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::AddInternalReader(const std::string& name, vtkAlgorithm* reader)
{
  if (reader)
  {
    ReaderPipeline pipe;
    pipe.Name = name;
    pipe.Reader = reader;
    pipe.PostPro->SetInputConnection(pipe.Reader->GetOutputPort());
    this->Pimpl->Readers.push_back(std::move(pipe));
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::RemoveInternalReaders()
{
  this->Pimpl->Readers.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::CanReadFile()
{
  return this->Pimpl->Readers.size() > 0 && this->Pimpl->Readers[0].Reader != nullptr;
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetOutputsDescription()
{
  std::string description;
  for (const ReaderPipeline& pipe : this->Pimpl->Readers)
  {
    if (this->Pimpl->Readers.size() > 0)
    {
      description += "=== " + pipe.Name + " ===\n";
    }
    description += pipe.OutputDescription;
  }
  return description;
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
std::string vtkF3DGenericImporter::GetMetaDataDescription()
{
  vtkIdType nPoints = 0;
  vtkIdType nCells = 0;
  for (ReaderPipeline& pipe : this->Pimpl->Readers)
  {
    vtkDataObject* object = pipe.Reader->GetOutputDataObject(0);
    if (object)
    {
      nPoints += object->GetNumberOfElements(vtkDataObject::POINT);
      nCells += object->GetNumberOfElements(vtkDataObject::CELL);
    }
  }

  std::string description;
  if (this->Pimpl->Readers.size() > 1)
  {
    description += "Number of geometries: ";
    description += std::to_string(this->Pimpl->Readers.size());
    description += "\n";
  }
  description += "Number of points: ";
  description += std::to_string(nPoints);
  description += "\n";
  description += "Number of cells: ";
  description += std::to_string(nCells);
  return description;
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::UpdateAtTimeValue(double timeValue)
{
  // Update each reader
  bool hasGeometry = false;
  for (ReaderPipeline& pipe : this->Pimpl->Readers)
  {
    if(!pipe.PostPro->UpdateTimeStep(timeValue) || !pipe.Reader->GetOutputDataObject(0))
    {
      F3DLog::Print(F3DLog::Severity::Warning, "A reader failed to update at a timeValue");
      pipe.Output = nullptr;
      pipe.Imported = false;
      continue;
    }
    hasGeometry = true;
  }

  this->UpdateColoringVectors(false);
  this->UpdateColoringVectors(true);
  this->UpdateOutputDescriptions();
  return hasGeometry;
}

//----------------------------------------------------------------------------
std::vector<std::pair<vtkActor*, vtkPolyDataMapper*>>
vtkF3DGenericImporter::GetGeometryActorsAndMappers()
{
  std::vector<std::pair<vtkActor*, vtkPolyDataMapper*>> actorsAndMappers(
    this->Pimpl->Readers.size());

  std::transform(this->Pimpl->Readers.cbegin(), this->Pimpl->Readers.cend(),
    actorsAndMappers.begin(),
    [](const ReaderPipeline& pipe)
    { return std::make_pair(pipe.GeometryActor.Get(), pipe.PolyDataMapper.Get()); });

  return actorsAndMappers;
}

//----------------------------------------------------------------------------
std::vector<std::pair<vtkActor*, vtkPointGaussianMapper*>>
vtkF3DGenericImporter::GetPointSpritesActorsAndMappers()
{
  std::vector<std::pair<vtkActor*, vtkPointGaussianMapper*>> actorsAndMappers(
    this->Pimpl->Readers.size());

  std::transform(this->Pimpl->Readers.cbegin(), this->Pimpl->Readers.cend(),
    actorsAndMappers.begin(),
    [](const ReaderPipeline& pipe)
    { return std::make_pair(pipe.PointSpritesActor.Get(), pipe.PointGaussianMapper.Get()); });

  return actorsAndMappers;
}

//----------------------------------------------------------------------------
std::vector<std::pair<vtkVolume*, vtkSmartVolumeMapper*>>
vtkF3DGenericImporter::GetVolumePropsAndMappers()
{
  std::vector<std::pair<vtkVolume*, vtkSmartVolumeMapper*>> propsAndMappers(
    this->Pimpl->Readers.size());

  std::transform(this->Pimpl->Readers.cbegin(), this->Pimpl->Readers.cend(),
    propsAndMappers.begin(),
    [](const ReaderPipeline& pipe)
    { return std::make_pair(pipe.VolumeProp.Get(), pipe.VolumeMapper.Get()); });

  return propsAndMappers;
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::UpdateColoringVectors(bool useCellData)
{
  // Recover all possible names
  std::set<std::string> arrayNames;
  for (ReaderPipeline& pipe : this->Pimpl->Readers)
  {
    if (!pipe.Output)
    {
      continue;
    }

    vtkDataSetAttributes* attr = useCellData
      ? static_cast<vtkDataSetAttributes*>(pipe.Output->GetCellData())
      : static_cast<vtkDataSetAttributes*>(pipe.Output->GetPointData());

    for (int i = 0; i < attr->GetNumberOfArrays(); i++)
    {
      vtkDataArray* array = attr->GetArray(i);
      if (array && array->GetName())
      {
        arrayNames.insert(array->GetName());
      }
    }
  }

  auto& data = useCellData ? this->Pimpl->CellDataArrayVectorForColoring
                           : this->Pimpl->PointDataArrayVectorForColoring;
  data.clear();

  // Create a vector of arrays by name
  for (const std::string& arrayName : arrayNames)
  {
    vtkF3DGenericImporter::ColoringInfo info;
    info.Name = arrayName;
    for (ReaderPipeline& pipe : this->Pimpl->Readers)
    {
      if (!pipe.Output)
      {
        continue;
      }

      vtkDataArray* array = useCellData ? pipe.Output->GetCellData()->GetArray(arrayName.c_str())
                                        : pipe.Output->GetPointData()->GetArray(arrayName.c_str());
      if (array)
      {
        info.MaximumNumberOfComponents =
          std::max(info.MaximumNumberOfComponents, array->GetNumberOfComponents());

        // Set ranges
        // XXX this does not take animation into account
        std::array<double, 2> range;
        array->GetRange(range.data(), -1);
        info.MagnitudeRange[0] = std::min(info.MagnitudeRange[0], range[0]);
        info.MagnitudeRange[1] = std::max(info.MagnitudeRange[1], range[1]);

        for (size_t i = 0; i < static_cast<size_t>(array->GetNumberOfComponents()); i++)
        {
          array->GetRange(range.data(), static_cast<int>(i));
          if (i < info.ComponentRanges.size())
          {
            info.ComponentRanges[i][0] = std::min(info.ComponentRanges[i][0], range[0]);
            info.ComponentRanges[i][1] = std::max(info.ComponentRanges[i][1], range[1]);
          }
          else
          {
            info.ComponentRanges.emplace_back(range);
          }
        }

        // Set component names
        if (array->HasAComponentName())
        {
          for (size_t i = 0; i < static_cast<size_t>(array->GetNumberOfComponents()); i++)
          {
            const char* compName = array->GetComponentName(i);
            if (i < info.ComponentNames.size())
            {
              if (compName && info.ComponentNames[i] != std::string(compName))
              {
                // set non-coherent component names to empty string
                info.ComponentNames[i] = "";
              }
            }
            else
            {
              // Add components names to the back of the component names vector
              info.ComponentNames.emplace_back(compName ? compName : "");
            }
          }
        }
      }
      info.Arrays.emplace_back(array);
    }

    data.emplace_back(info);
  }
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::GetInfoForColoring(
  bool useCellData, int index, vtkF3DGenericImporter::ColoringInfo& info)
{
  auto& data = useCellData ? this->Pimpl->CellDataArrayVectorForColoring
                           : this->Pimpl->PointDataArrayVectorForColoring;

  if (index < 0 || index >= static_cast<int>(data.size()))
  {
    return false;
  }
  info = data[index];
  return true;
}

//----------------------------------------------------------------------------
int vtkF3DGenericImporter::GetNumberOfIndexesForColoring(bool useCellData)
{
  auto& data = useCellData ? this->Pimpl->CellDataArrayVectorForColoring
                           : this->Pimpl->PointDataArrayVectorForColoring;
  return static_cast<int>(data.size());
}

//----------------------------------------------------------------------------
int vtkF3DGenericImporter::FindIndexForColoring(bool useCellData, const std::string& arrayName)
{
  auto& data = useCellData ? this->Pimpl->CellDataArrayVectorForColoring
                           : this->Pimpl->PointDataArrayVectorForColoring;
  for (size_t i = 0; i < data.size(); i++)
  {
    if (data[i].Name == arrayName)
    {
      return static_cast<int>(i);
    }
  }
  return -1;
}

//----------------------------------------------------------------------------
const vtkBoundingBox& vtkF3DGenericImporter::GetGeometryBoundingBox()
{
  return this->Pimpl->GeometryBoundingBox;
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::UpdateOutputDescriptions()
{
  for (ReaderPipeline& pipe : this->Pimpl->Readers)
  {
    if (pipe.Imported)
    {
      // Recover output description
      vtkDataObject* readerOutput = pipe.Reader->GetOutputDataObject(0);
      pipe.OutputDescription = vtkF3DGenericImporter::GetDataObjectDescription(readerOutput);
    }
  }
}
