#include "vtkF3DGenericImporter.h"

#include "F3DLog.h"

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkCallbackCommand.h>
#include <vtkCellData.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDataObject.h>
#include <vtkDoubleArray.h>
#include <vtkEventForwarderCommand.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageToPoints.h>
#include <vtkInformation.h>
#include <vtkLightKit.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRectilinearGrid.h>
#include <vtkRectilinearGridToPointSet.h>
#include <vtkRenderer.h>
#include <vtkScalarsToColors.h>
#include <vtkSmartPointer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTexture.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkVolumeProperty.h>
#include <vtksys/SystemTools.hxx>
#include <vtkPointData.h>
#include <vtkCellData.h>

#include <sstream>

namespace{
//----------------------------------------------------------------------------
// TODO : add this function in a utils file for rendering in VTK directly
vtkSmartPointer<vtkTexture> GetTexture(
  const std::string& filePath, bool isSRGB = false)
{
  vtkSmartPointer<vtkTexture> texture;
  if (!filePath.empty())
  {
    std::string fullPath = vtksys::SystemTools::CollapseFullPath(filePath);
    if (!vtksys::SystemTools::FileExists(fullPath))
    {
      F3DLog::Print(F3DLog::Severity::Warning, "Texture file does not exist " + fullPath + "\n");
    }
    else
    {
      auto reader = vtkSmartPointer<vtkImageReader2>::Take(
        vtkImageReader2Factory::CreateImageReader2(fullPath.c_str()));
      if (reader)
      {
        reader->SetFileName(fullPath.c_str());
        reader->Update();
        texture = vtkSmartPointer<vtkTexture>::New();
        texture->SetInputConnection(reader->GetOutputPort());
        if (isSRGB)
        {
          texture->UseSRGBColorSpaceOn();
        }
        texture->InterpolateOn();
        return texture;
      }
      else
      {
        F3DLog::Print(F3DLog::Severity::Warning, "Cannot open texture file " + fullPath + "\n");
      }
    }
  }

  return texture;
}
}

struct ReaderPipeline
{
  ReaderPipeline()
  {
    this->GeometryActor->GetProperty()->SetInterpolationToPBR();
    this->VolumeMapper->SetRequestedRenderModeToGPU();
    this->PolyDataMapper->InterpolateScalarsBeforeMappingOn();
    this->PointGaussianMapper->EmissiveOff();
    this->PointGaussianMapper->SetSplatShaderCode(
      "//VTK::Color::Impl\n"
      "float dist = dot(offsetVCVSOutput.xy, offsetVCVSOutput.xy);\n"
      "if (dist > 1.0) {\n"
      "  discard;\n"
      "} else {\n"
      "  float scale = (1.0 - dist);\n"
      "  ambientColor *= scale;\n"
      "  diffuseColor *= scale;\n"
      "}\n");
  }

  std::string Name;
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

struct vtkF3DGenericImporter::vtkInternals
{
  std::vector<ReaderPipeline> Readers;

  std::vector<ColoringInfo> PointDataArrayVectorForColoring;
  std::vector<ColoringInfo> CellDataArrayVectorForColoring;
  vtkBoundingBox GeometryBoundingBox;
  std::string MetaDataDescription;

  bool AnimationEnabled = false;
  std::set<double> TimeSteps;
};

vtkStandardNewMacro(vtkF3DGenericImporter);

//----------------------------------------------------------------------------
vtkF3DGenericImporter::vtkF3DGenericImporter()
  : Internals(new vtkF3DGenericImporter::vtkInternals())
{
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::UpdateTemporalInformation()
{
  this->Internals->TimeSteps.clear();

  // Update each reader
  for (ReaderPipeline& pipe : this->Internals->Readers)
  {
    pipe.Reader->UpdateInformation();
    vtkInformation* readerInfo = pipe.Reader->GetOutputInformation(0);

    int nbTimeSteps = readerInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
    double* readerTimeSteps = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
    for (int i = 0; i < nbTimeSteps; i++)
    {
      this->Internals->TimeSteps.insert(readerTimeSteps[i]);
    }
  }
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DGenericImporter::GetNumberOfAnimations()
{
  return this->Internals->TimeSteps.size() > 0 ? 1 : 0;
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
    this->Internals->AnimationEnabled = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::DisableAnimation(vtkIdType animationIndex)
{
  if (animationIndex < this->GetNumberOfAnimations())
  {
    this->Internals->AnimationEnabled = false;
  }
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::IsAnimationEnabled(vtkIdType animationIndex)
{
  return animationIndex < this->GetNumberOfAnimations() ? this->Internals->AnimationEnabled : false;
}

//----------------------------------------------------------------------------
// Complete GetTemporalInformation needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7246
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20201016)
bool vtkF3DGenericImporter::GetTemporalInformation(vtkIdType animationIndex,
  double vtkNotUsed(frameRate), int& nbTimeSteps, double timeRange[2], vtkDoubleArray* timeSteps)
#else
bool vtkF3DGenericImporter::GetTemporalInformation(
  vtkIdType animationIndex, int& nbTimeSteps, double timeRange[2], vtkDoubleArray* timeSteps)
#endif
{
  if (animationIndex < this->GetNumberOfAnimations())
  {
    nbTimeSteps = static_cast<int>(this->Internals->TimeSteps.size());
    timeRange[0] = *this->Internals->TimeSteps.cbegin();
    timeRange[1] = *this->Internals->TimeSteps.crbegin();

    for (double ts : this->Internals->TimeSteps)
    {
      timeSteps->InsertNextValue(ts);
    }
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportActors(vtkRenderer* ren)
{
  this->Internals->GeometryBoundingBox.Reset();

  // Update each reader
  for (size_t readerIndex = 0; readerIndex < this->Internals->Readers.size(); readerIndex++)
  {
    ReaderPipeline& pipe = this->Internals->Readers[readerIndex];

    {
    // Forward progress event
    vtkNew<vtkCallbackCommand> progressCallback;
    double progressRatio = static_cast<double>(readerIndex + 1) / this->Internals->Readers.size();
    std::pair<vtkF3DGenericImporter*, double> progressPair = {this, progressRatio};
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
    pipe.PostPro->Update();
    pipe.Reader->RemoveObservers(vtkCommand::ProgressEvent);
    }

    // Recover output
    vtkDataObject* readerOutput = pipe.Reader->GetOutputDataObject(0);
    if (!readerOutput)
    {
      F3DLog::Print(F3DLog::Severity::Warning, "A reader did not produce any output");
      continue;
    }

    // Cast to dataset types
    vtkPolyData* surface = vtkPolyData::SafeDownCast(pipe.PostPro->GetOutput());
    vtkImageData* image = vtkImageData::SafeDownCast(pipe.PostPro->GetOutput(2));
    vtkDataSet* dataSet = vtkImageData::SafeDownCast(pipe.PostPro->GetInput())
      ? vtkDataSet::SafeDownCast(image)
      : vtkDataSet::SafeDownCast(surface);
    pipe.Output = dataSet;
    
    // Recover output description
    pipe.OutputDescription = vtkF3DGenericImporter::GetDataObjectDescription(readerOutput);

    // Recover data for coloring
    pipe.PointDataForColoring = vtkDataSetAttributes::SafeDownCast(dataSet->GetPointData());
    pipe.CellDataForColoring = vtkDataSetAttributes::SafeDownCast(dataSet->GetCellData());

    // Increase bounding box size if needed
    double bounds[6];
    surface->GetBounds(bounds);
    this->Internals->GeometryBoundingBox.AddBounds(bounds);

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
    pipe.GeometryActor->SetVisibility(false);
    pipe.PointSpritesActor->SetVisibility(false);
    pipe.VolumeProp->SetVisibility(false);

    // TODO move to renderer
    pipe.GeometryActor->GetProperty()->SetColor(this->SurfaceColor);
    pipe.GeometryActor->GetProperty()->SetOpacity(this->Opacity);
    pipe.GeometryActor->GetProperty()->SetRoughness(this->Roughness);
    pipe.GeometryActor->GetProperty()->SetMetallic(this->Metallic);
    pipe.GeometryActor->GetProperty()->SetLineWidth(this->LineWidth);
    pipe.GeometryActor->GetProperty()->SetPointSize(this->PointSize);

    pipe.PointSpritesActor->GetProperty()->SetColor(this->SurfaceColor);
    pipe.PointSpritesActor->GetProperty()->SetOpacity(this->Opacity);

    // Textures
    auto colorTex = ::GetTexture(this->TextureBaseColor, true);
    pipe.GeometryActor->GetProperty()->SetBaseColorTexture(colorTex);
    pipe.GeometryActor->GetProperty()->SetORMTexture(::GetTexture(this->TextureMaterial));
    pipe.GeometryActor->GetProperty()->SetEmissiveTexture(
      ::GetTexture(this->TextureEmissive, true));
    pipe.GeometryActor->GetProperty()->SetEmissiveFactor(this->EmissiveFactor);
    pipe.GeometryActor->GetProperty()->SetNormalTexture(::GetTexture(this->TextureNormal));
    pipe.GeometryActor->GetProperty()->SetNormalScale(this->NormalScale);

    // If the input texture is RGBA, flag the actor as translucent
    if (colorTex && colorTex->GetImageDataInput(0)->GetNumberOfScalarComponents() == 4)
    {
      pipe.GeometryActor->ForceTranslucentOn();
    }

  }
  this->UpdateTemporalInformation();
  this->UpdateColoringVectors(false);
  this->UpdateColoringVectors(true);
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::PrintSelf(std::ostream& os, vtkIndent indent)
{
  vtkImporter::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::AddInternalReader(const std::string& name, vtkSmartPointer<vtkAlgorithm> reader)
{
  if (reader)
  {
    ReaderPipeline pipe;
    pipe.Name = name;
    pipe.Reader = reader;
    pipe.PostPro->SetInputConnection(pipe.Reader->GetOutputPort());
    this->Internals->Readers.push_back(std::move(pipe));
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::RemoveInternalReaders()
{
  this->Internals->Readers.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::CanReadFile()
{
  return this->Internals->Readers.size() > 0 && this->Internals->Readers[0].Reader != nullptr;
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetOutputsDescription()
{
  std::string description;
  for (ReaderPipeline& pipe : this->Internals->Readers)
  {
    if (this->Internals->Readers.size() > 0)
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
std::string vtkF3DGenericImporter::GetDataObjectDescription(vtkDataObject* object)
{
  vtkMultiBlockDataSet* mb = vtkMultiBlockDataSet::SafeDownCast(object);
  vtkDataSet* ds = vtkDataSet::SafeDownCast(object);
  if (mb)
  {
    return vtkF3DGenericImporter::GetMultiBlockDescription(mb, vtkIndent(0));
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
  for (ReaderPipeline& pipe : this->Internals->Readers)
  {
    vtkDataObject* object = pipe.Reader->GetOutputDataObject(0);
    if (object)
    {
      nPoints += object->GetNumberOfElements(vtkDataObject::POINT);
      nCells += object->GetNumberOfElements(vtkDataObject::CELL);
    }
  }

  std::string description;
  if (this->Internals->Readers.size() > 1)
  {
    description += "Number of geometries: ";
    description += std::to_string(this->Internals->Readers.size());
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
void vtkF3DGenericImporter::UpdateTimeStep(double timestep)
{
  // Update each reader
  for (ReaderPipeline& pipe : this->Internals->Readers)
  {
    pipe.PostPro->UpdateTimeStep(timestep);
  }
  this->UpdateColoringVectors(false);
  this->UpdateColoringVectors(true);
}

//----------------------------------------------------------------------------
std::vector<std::pair<vtkActor*, vtkPolyDataMapper*> > vtkF3DGenericImporter::GetGeometryActorsAndMappers()
{
  std::vector<std::pair<vtkActor*, vtkPolyDataMapper*> > actorsAndMappers;
  for (ReaderPipeline& pipe : this->Internals->Readers)
  {
    actorsAndMappers.emplace_back(std::make_pair(pipe.GeometryActor.Get(), pipe.PolyDataMapper.Get()));
  }
  return actorsAndMappers;
}

//----------------------------------------------------------------------------
std::vector<std::pair<vtkActor*, vtkPointGaussianMapper*> > vtkF3DGenericImporter::GetPointSpritesActorsAndMappers()
{
  std::vector<std::pair<vtkActor*, vtkPointGaussianMapper*> > actorsAndMappers;
  for (ReaderPipeline& pipe : this->Internals->Readers)
  {
    actorsAndMappers.emplace_back(std::make_pair(pipe.PointSpritesActor.Get(), pipe.PointGaussianMapper.Get()));
  }
  return actorsAndMappers;
}

//----------------------------------------------------------------------------
std::vector<std::pair<vtkVolume*, vtkSmartVolumeMapper*> > vtkF3DGenericImporter::GetVolumePropsAndMappers()
{
  std::vector<std::pair<vtkVolume*, vtkSmartVolumeMapper*> > propsAndMappers;
  for (ReaderPipeline& pipe : this->Internals->Readers)
  {
    propsAndMappers.emplace_back(std::make_pair(pipe.VolumeProp.Get(), pipe.VolumeMapper.Get()));
  }
  return propsAndMappers;
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::UpdateColoringVectors(bool useCellData)
{
  // Recover all possible names
  std::set<std::string> arrayNames;
  for (ReaderPipeline& pipe : this->Internals->Readers)
  {
    vtkDataSetAttributes* attr = useCellData
      ? static_cast<vtkDataSetAttributes*>(pipe.Output->GetCellData())
      : static_cast<vtkDataSetAttributes*>(pipe.Output->GetPointData());

    for (int i = 0; i < attr->GetNumberOfArrays(); i++)
    {
      vtkDataArray* array = attr->GetArray(i);
      if (array)
      {
        arrayNames.insert(array->GetName());
      }
    }
  }

  auto& data = useCellData
    ? this->Internals->CellDataArrayVectorForColoring
    : this->Internals->PointDataArrayVectorForColoring;
  data.clear();

  // Create a vector of arrays by name
  for (std::string arrayName : arrayNames)
  {
    vtkF3DGenericImporter::ColoringInfo info;
    info.Name = arrayName;
    for (ReaderPipeline& pipe : this->Internals->Readers)
    {
      vtkDataArray* array = useCellData
        ? pipe.Output->GetCellData()->GetArray(arrayName.c_str())
        : pipe.Output->GetPointData()->GetArray(arrayName.c_str());
      if (array)
      {
        info.MaximumNumberOfComponents = std::max(info.MaximumNumberOfComponents, array->GetNumberOfComponents());

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
bool vtkF3DGenericImporter::GetInfoForColoring(bool useCellData, int index, vtkF3DGenericImporter::ColoringInfo& info)
{
  auto& data = useCellData
    ? this->Internals->CellDataArrayVectorForColoring
    : this->Internals->PointDataArrayVectorForColoring;

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
  auto& data = useCellData
    ? this->Internals->CellDataArrayVectorForColoring
    : this->Internals->PointDataArrayVectorForColoring;
  return static_cast<int> (data.size());
}

//----------------------------------------------------------------------------
int vtkF3DGenericImporter::FindIndexForColoring(bool useCellData, std::string arrayName)
{
  auto& data = useCellData
    ? this->Internals->CellDataArrayVectorForColoring
    : this->Internals->PointDataArrayVectorForColoring;
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
  return this->Internals->GeometryBoundingBox;
}
