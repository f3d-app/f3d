#include "vtkF3DGenericImporter.h"

#include "F3DLog.h"
#include "vtkF3DPostProcessFilter.h"

#include <vtkActor.h>
#include <vtkCompositeDataIterator.h>
#include <vtkDataAssembly.h>
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
  // Data structure for each block in a composite dataset
  struct BlockData
  {
    vtkNew<vtkF3DPostProcessFilter> PostPro;
    vtkNew<vtkActor> Actor;
    vtkNew<vtkPolyDataMapper> Mapper;
    vtkPolyData* Points = nullptr;
    vtkImageData* Image = nullptr;
    std::string Name;
  };

  vtkSmartPointer<vtkAlgorithm> Reader = nullptr;
  std::vector<BlockData> Blocks;
  std::string OutputDescription;

  bool HasAnimation = false;
  bool AnimationEnabled = false;
  std::array<double, 2> TimeRange;

  void UpdateBlock(BlockData& bd, vtkDataSet* dataset)
  {
    bd.PostPro->SetInputDataObject(dataset);
    bd.PostPro->Update();
    bd.Points = vtkPolyData::SafeDownCast(bd.PostPro->GetOutput(1));
    vtkImageData* image = vtkImageData::SafeDownCast(bd.PostPro->GetOutput(2));
    bd.Image = image && image->GetNumberOfCells() > 0 ? image : nullptr;
  }
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
  double timeRange[2], int& vtkNotUsed(nbTimeSteps), vtkDoubleArray* vtkNotUsed(timeSteps))
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
void vtkF3DGenericImporter::CreateActorForBlock(
  vtkDataSet* block, vtkRenderer* ren, const std::string& blockName)
{
  this->Pimpl->Blocks.emplace_back();
  Internals::BlockData& bd = this->Pimpl->Blocks.back();
  bd.Name = blockName;

  bd.PostPro->SetInputDataObject(block);
  bd.PostPro->Update();

  bd.Points = vtkPolyData::SafeDownCast(bd.PostPro->GetOutput(1));
  vtkImageData* image = vtkImageData::SafeDownCast(bd.PostPro->GetOutput(2));
  bd.Image = (image && image->GetNumberOfCells() > 0) ? image : nullptr;

  bd.Mapper->SetInputConnection(bd.PostPro->GetOutputPort(0));
  bd.Mapper->ScalarVisibilityOff();

  bd.Actor->SetMapper(bd.Mapper);
  bd.Actor->GetProperty()->SetPointSize(10.0);
  bd.Actor->GetProperty()->SetLineWidth(1.0);
  bd.Actor->GetProperty()->SetRoughness(0.3);
  bd.Actor->GetProperty()->SetBaseIOR(1.5);
  bd.Actor->GetProperty()->SetInterpolationToPBR();

  ren->AddActor(bd.Actor);
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  this->ActorCollection->AddItem(bd.Actor);
#endif

  bd.Actor->VisibilityOn();
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportActors(vtkRenderer* ren)
{
  assert(this->Pimpl->Reader);

  // Clear any previous blocks
  this->Pimpl->Blocks.clear();

  // Read file and forward progress
  vtkNew<vtkEventForwarderCommand> progressForwarder;
  progressForwarder->SetTarget(this);
  this->Pimpl->Reader->AddObserver(vtkCommand::ProgressEvent, progressForwarder);
  bool status = this->Pimpl->Reader->GetExecutive()->Update();

  vtkDataObject* output = this->Pimpl->Reader->GetOutputDataObject(0);
  if (!status || !output)
  {
    this->SetFailureStatus();
    return;
  }

  this->Pimpl->OutputDescription = this->GetDataObjectDescription(output);

  vtkMultiBlockDataSet* mb = vtkMultiBlockDataSet::SafeDownCast(output);
  vtkPartitionedDataSetCollection* pdc = vtkPartitionedDataSetCollection::SafeDownCast(output);
  vtkPartitionedDataSet* pds = vtkPartitionedDataSet::SafeDownCast(output);

  if (mb)
  {
    this->ImportMultiBlock(mb, ren);
  }
  else if (pdc)
  {
    this->ImportPartitionedDataSetCollection(pdc, ren);
  }
  else if (pds)
  {
    this->ImportPartitionedDataSet(pds, ren);
  }
#if VTK_VERSION_NUMBER <= VTK_VERSION_CHECK(9, 5, 2)
  // Handle other composite types (e.g., AMR) using generic iterator
  else if (vtkCompositeDataSet* composite = vtkCompositeDataSet::SafeDownCast(output))
  {
    auto iter = vtkSmartPointer<vtkCompositeDataIterator>::Take(composite->NewIterator());
    iter->SkipEmptyNodesOn();
    int idx = 0;
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem(), idx++)
    {
      vtkDataSet* ds = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (ds)
      {
        std::string blockName = "Block_" + std::to_string(idx);
        if (iter->HasCurrentMetaData())
        {
          const char* name = iter->GetCurrentMetaData()->Get(vtkCompositeDataSet::NAME());
          if (name)
          {
            blockName = name;
          }
        }
        this->CreateActorForBlock(ds, ren, blockName);
      }
    }
  }
#endif
  else
  {
    vtkDataSet* dataset = vtkDataSet::SafeDownCast(output);
    if (dataset)
    {
      this->CreateActorForBlock(dataset, ren, "");
    }
    else
    {
      this->SetFailureStatus();
      return;
    }
  }

  this->UpdateTemporalInformation();
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::SetInternalReader(vtkAlgorithm* reader)
{
  if (reader)
  {
    this->Pimpl->Reader = reader;
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

  vtkInformation* info = this->Pimpl->Reader->GetOutputInformation(0);
  info->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), timeValue);
  bool status = this->Pimpl->Reader->GetExecutive()->Update();

  vtkDataObject* output = this->Pimpl->Reader->GetOutputDataObject(0);
  if (!status || !output)
  {
    F3DLog::Print(F3DLog::Severity::Warning, "A reader failed to update at a timeValue");
    return false;
  }

  vtkCompositeDataSet* composite = vtkCompositeDataSet::SafeDownCast(output);

  if (composite)
  {
    auto iter = vtkSmartPointer<vtkCompositeDataIterator>::Take(composite->NewIterator());
    iter->SkipEmptyNodesOn();

    size_t blockIdx = 0;
    for (iter->InitTraversal();
         !iter->IsDoneWithTraversal() && blockIdx < this->Pimpl->Blocks.size();
         iter->GoToNextItem(), blockIdx++)
    {
      vtkDataSet* block = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      if (block)
      {
        this->Pimpl->UpdateBlock(this->Pimpl->Blocks[blockIdx], block);
      }
    }
  }
  else if (!this->Pimpl->Blocks.empty())
  {
    vtkDataSet* dataset = vtkDataSet::SafeDownCast(output);
    if (dataset)
    {
      this->Pimpl->UpdateBlock(this->Pimpl->Blocks[0], dataset);
    }
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
vtkPolyData* vtkF3DGenericImporter::GetImportedPoints(vtkIdType actorIndex)
{
  if (actorIndex >= 0 && actorIndex < static_cast<vtkIdType>(this->Pimpl->Blocks.size()))
  {
    return this->Pimpl->Blocks[actorIndex].Points;
  }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkImageData* vtkF3DGenericImporter::GetImportedImage(vtkIdType actorIndex)
{
  if (actorIndex >= 0 && actorIndex < static_cast<vtkIdType>(this->Pimpl->Blocks.size()))
  {
    return this->Pimpl->Blocks[actorIndex].Image;
  }
  return nullptr;
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetBlockName(vtkIdType actorIndex)
{
  if (actorIndex >= 0 && actorIndex < static_cast<vtkIdType>(this->Pimpl->Blocks.size()))
  {
    return this->Pimpl->Blocks[actorIndex].Name;
  }
  return "";
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DGenericImporter::GetNumberOfBlocks()
{
  return static_cast<vtkIdType>(this->Pimpl->Blocks.size());
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportMultiBlock(
  vtkMultiBlockDataSet* mb, vtkRenderer* ren, const std::string& parentName)
{
  for (unsigned int i = 0; i < mb->GetNumberOfBlocks(); i++)
  {
    vtkDataObject* obj = mb->GetBlock(i);
    if (!obj)
    {
      continue;
    }

    std::string blockName = parentName;
    if (!blockName.empty())
    {
      blockName += "/";
    }

    if (const char* name =
          mb->HasMetaData(i) ? mb->GetMetaData(i)->Get(vtkCompositeDataSet::NAME()) : nullptr)
    {
      blockName += name;
    }
    else
    {
      blockName += "Block_" + std::to_string(i);
    }

    vtkMultiBlockDataSet* childMB = vtkMultiBlockDataSet::SafeDownCast(obj);
    vtkCompositeDataSet* childComposite = vtkCompositeDataSet::SafeDownCast(obj);
    vtkDataSet* ds = vtkDataSet::SafeDownCast(obj);

    if (childMB)
    {
      this->ImportMultiBlock(childMB, ren, blockName);
    }
    else if (childComposite)
    {
      auto iter = vtkSmartPointer<vtkCompositeDataIterator>::Take(childComposite->NewIterator());
      iter->SkipEmptyNodesOn();
      int subIdx = 0;
      for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem(), subIdx++)
      {
        vtkDataSet* subDs = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
        if (subDs)
        {
          const char* name = iter->HasCurrentMetaData()
            ? iter->GetCurrentMetaData()->Get(vtkCompositeDataSet::NAME())
            : "Block_";
          blockName += "/" + (name ? std::string(name) : "Block_") + std::to_string(subIdx);
          this->CreateActorForBlock(subDs, ren, blockName);
        }
      }
    }
    else if (ds)
    {
      this->CreateActorForBlock(ds, ren, blockName);
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportPartitionedDataSetCollection(
  vtkPartitionedDataSetCollection* pdc, vtkRenderer* ren)
{
  std::map<unsigned int, std::string> datasetIndexToName;
  vtkDataAssembly* assembly = pdc->GetDataAssembly();
  if (assembly)
  {
    std::vector<int> childNodes = assembly->GetChildNodes(assembly->GetRootNode());
    for (int nodeId : childNodes)
    {
      const char* nodeName = assembly->GetNodeName(nodeId);
      if (nodeName)
      {
        std::vector<unsigned int> indices = assembly->GetDataSetIndices(nodeId, false);
        for (unsigned int idx : indices)
        {
          datasetIndexToName[idx] = nodeName;
        }
      }
    }
  }

  for (unsigned int i = 0; i < pdc->GetNumberOfPartitionedDataSets(); i++)
  {
    vtkPartitionedDataSet* pds = pdc->GetPartitionedDataSet(i);
    if (!pds)
    {
      continue;
    }

    std::string pdsName;

    auto it = datasetIndexToName.find(i);
    if (it != datasetIndexToName.end())
    {
      pdsName = it->second;
    }
    else if (pdc->HasMetaData(i))
    {
      const char* name = pdc->GetMetaData(i)->Get(vtkCompositeDataSet::NAME());
      if (name)
      {
        pdsName = name;
      }
    }
    if (pdsName.empty())
    {
      pdsName = "PartitionedDataSet_" + std::to_string(i);
    }

    this->ImportPartitionedDataSet(pds, ren, pdsName);
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportPartitionedDataSet(
  vtkPartitionedDataSet* pds, vtkRenderer* ren, const std::string& pdsName)
{
  std::string baseName = pdsName;
  if (baseName.empty())
  {
    baseName = "PartitionedDataSet_" + std::to_string(this->Pimpl->Blocks.size());
  }

  for (unsigned int j = 0; j < pds->GetNumberOfPartitions(); j++)
  {
    vtkDataSet* ds = pds->GetPartition(j);
    if (!ds)
    {
      continue;
    }

    std::string partitionName;
    if (pds->HasMetaData(j))
    {
      const char* name = pds->GetMetaData(j)->Get(vtkCompositeDataSet::NAME());
      if (name)
      {
        partitionName = name;
      }
    }

    std::string blockName = baseName;
    if (!partitionName.empty())
    {
      blockName += "/";
      blockName += partitionName;
    }
    else if (pds->GetNumberOfPartitions() > 1)
    {
      blockName += "/Partition_";
      blockName += std::to_string(j);
    }

    this->CreateActorForBlock(ds, ren, blockName);
  }
}
