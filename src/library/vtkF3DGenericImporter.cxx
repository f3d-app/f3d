#include "vtkF3DGenericImporter.h"

#include "F3DConfig.h"
#include "f3d_options.h"
#include "vtkF3DLog.h"

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkBoundingBox.h>
#include <vtkCellData.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkDataSetSurfaceFilter.h>
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
#include <vtkScalarBarActor.h>
#include <vtkScalarsToColors.h>
#include <vtkSmartPointer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTexture.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkVolumeProperty.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkF3DGenericImporter);

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::UpdateTemporalInformation()
{
  if (!this->TemporalInformationUpdated)
  {
    if (!this->Reader->IsReaderValid())
    {
      vtkF3DLog::Print(vtkF3DLog::Severity::Warning, "Reader is not valid");
      return;
    }
    this->Reader->UpdateInformation();
    vtkInformation* readerInfo = this->Reader->GetOutputInformation(0);

    this->NbTimeSteps = readerInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
    this->TimeRange = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE());
    this->TimeSteps = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
    this->TemporalInformationUpdated = true;
  }
}

//----------------------------------------------------------------------------
vtkIdType vtkF3DGenericImporter::GetNumberOfAnimations()
{
  this->UpdateTemporalInformation();
  return this->NbTimeSteps > 0 ? 1 : 0;
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
    this->AnimationEnabled = true;
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::DisableAnimation(vtkIdType animationIndex)
{
  if (animationIndex < this->GetNumberOfAnimations())
  {
    this->AnimationEnabled = false;
  }
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::IsAnimationEnabled(vtkIdType animationIndex)
{
  return animationIndex < this->GetNumberOfAnimations() ? this->AnimationEnabled : false;
}

//----------------------------------------------------------------------------
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
    nbTimeSteps = this->NbTimeSteps;
    timeRange[0] = this->TimeRange[0];
    timeRange[1] = this->TimeRange[1];
    timeSteps->SetArray(this->TimeSteps, this->NbTimeSteps, 1);
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportActors(vtkRenderer* ren)
{
  if (!this->Reader->IsReaderValid())
  {
    vtkF3DLog::Print(vtkF3DLog::Severity::Error,
      std::string("File '") + this->Reader->GetFileName() + "' cannot be read.");
    return;
  }

  // forward progress event
  vtkNew<vtkEventForwarderCommand> forwarder;
  forwarder->SetTarget(this);
  this->Reader->AddObserver(vtkCommand::ProgressEvent, forwarder);

  this->PostPro->SetInputConnection(this->Reader->GetOutputPort());
  bool ret = this->PostPro->GetExecutive()->Update();

  if (!ret)
  {
    vtkF3DLog::Print(vtkF3DLog::Severity::Error,
      std::string("File '") + this->Reader->GetFileName() + "' cannot be read.");
    return;
  }

  this->OutputDescription =
    vtkF3DGenericImporter::GetDataObjectDescription(this->Reader->GetOutput());

  if (!this->GetRenderWindow())
  {
    // No render window provided, do not setup the mappers and actors
    return;
  }

  vtkPolyData* surface = vtkPolyData::SafeDownCast(this->PostPro->GetOutput());
  vtkImageData* image = vtkImageData::SafeDownCast(this->PostPro->GetOutput(2));

  // Configure volume mapper
  this->VolumeMapper->SetInputConnection(this->PostPro->GetOutputPort(2));
  this->VolumeMapper->SetRequestedRenderModeToGPU();

  // Configure polydata mapper
  this->PolyDataMapper->InterpolateScalarsBeforeMappingOn();
  this->PolyDataMapper->SetInputConnection(this->PostPro->GetOutputPort(0));

  // Configure Point Gaussian mapper
  double bounds[6];
  surface->GetBounds(bounds);
  vtkBoundingBox bbox(bounds);

  double pointSize = this->Options->getAsDouble("point-size");
  double gaussianPointSize = 1.0;
  if (bbox.IsValid())
  {
    gaussianPointSize = pointSize * bbox.GetDiagonalLength() * 0.001;
  }
  this->PointGaussianMapper->SetInputConnection(this->PostPro->GetOutputPort(1));
  this->PointGaussianMapper->SetScaleFactor(gaussianPointSize);
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

  //
  vtkDataSet* dataSet = vtkImageData::SafeDownCast(this->PostPro->GetInput())
    ? vtkDataSet::SafeDownCast(image)
    : vtkDataSet::SafeDownCast(surface);

  std::string usedArray = this->Options->getAsString("scalars");
  this->PointDataForColoring = vtkDataSetAttributes::SafeDownCast(dataSet->GetPointData());
  this->CellDataForColoring = vtkDataSetAttributes::SafeDownCast(dataSet->GetCellData());
  vtkDataSetAttributes* dataForColoring =
    this->Options->getAsBool("cells") ? this->CellDataForColoring : this->PointDataForColoring;

  // Recover an array for coloring if we ever need it
  this->ArrayIndexForColoring = -1;
  if (usedArray.empty())
  {
    vtkDataArray* array = dataForColoring->GetScalars();
    if (array)
    {
      const char* arrayName = array->GetName();
      if (arrayName)
      {
        usedArray = arrayName;
      }
      this->OutputDescription += "\nUsing default scalar array: ";
      this->OutputDescription += usedArray;
    }
    else
    {
      for (int i = 0; i < dataForColoring->GetNumberOfArrays(); i++)
      {
        array = dataForColoring->GetArray(i);
        if (array)
        {
          this->ArrayIndexForColoring = i;
          const char* arrayName = array->GetName();
          if (arrayName)
          {
            usedArray = arrayName;
          }
          this->OutputDescription += "\nUsing first found array: ";
          this->OutputDescription += usedArray;
          break;
        }
      }
    }
  }
  if (this->ArrayIndexForColoring == -1)
  {
    dataForColoring->GetArray(usedArray.c_str(), this->ArrayIndexForColoring);
  }
  if (this->ArrayIndexForColoring == -1 && !usedArray.empty() &&
    usedArray != f3d::F3DReservedString)
  {
    vtkF3DLog::Print(vtkF3DLog::Severity::Warning, "Unknown scalar array: " + usedArray);
  }
  if (this->ArrayIndexForColoring == -1)
  {
    this->OutputDescription += "\nNo array found for scalar coloring and volume rendering";
  }

  // configure props
  this->VolumeProp->SetMapper(this->VolumeMapper);

  this->GeometryActor->SetMapper(this->PolyDataMapper);
  this->GeometryActor->GetProperty()->SetInterpolationToPBR();

  std::vector<double> color = this->Options->getAsDoubleVector("color");
  double opacity = this->Options->getAsDouble("opacity");
  this->GeometryActor->GetProperty()->SetColor(color.data());
  this->GeometryActor->GetProperty()->SetOpacity(opacity);
  this->GeometryActor->GetProperty()->SetRoughness(this->Options->getAsDouble("roughness"));
  this->GeometryActor->GetProperty()->SetMetallic(this->Options->getAsDouble("metallic"));
  this->GeometryActor->GetProperty()->SetLineWidth(this->Options->getAsDouble("line-width"));
  this->GeometryActor->GetProperty()->SetPointSize(pointSize);

  this->PointSpritesActor->SetMapper(this->PointGaussianMapper);
  this->PointSpritesActor->GetProperty()->SetColor(color.data());
  this->PointSpritesActor->GetProperty()->SetOpacity(opacity);

  // Textures
  this->GeometryActor->GetProperty()->SetBaseColorTexture(
    this->GetTexture(this->Options->getAsString("texture-base-color"), true));
  this->GeometryActor->GetProperty()->SetORMTexture(
    this->GetTexture(this->Options->getAsString("texture-material")));
  this->GeometryActor->GetProperty()->SetEmissiveTexture(
    this->GetTexture(this->Options->getAsString("texture-emissive"), true));
  this->GeometryActor->GetProperty()->SetEmissiveFactor(
    this->Options->getAsDoubleVector("emissive-factor").data());
  this->GeometryActor->GetProperty()->SetNormalTexture(
    this->GetTexture(this->Options->getAsString("texture-normal")));
  this->GeometryActor->GetProperty()->SetNormalScale(this->Options->getAsDouble("normal-scale"));

  // add props
  ren->AddActor2D(this->ScalarBarActor);
  ren->AddActor(this->GeometryActor);
  ren->AddActor(this->PointSpritesActor);
  ren->AddVolume(this->VolumeProp);
}

//----------------------------------------------------------------------------
// TODO : add this function in a utils file for rendering in VTK directly
vtkSmartPointer<vtkTexture> vtkF3DGenericImporter::GetTexture(
  const std::string& filePath, bool isSRGB)
{
  vtkSmartPointer<vtkTexture> texture;
  if (!filePath.empty())
  {
    std::string fullPath = vtksys::SystemTools::CollapseFullPath(filePath);
    if (!vtksys::SystemTools::FileExists(fullPath))
    {
      vtkF3DLog::Print(vtkF3DLog::Severity::Warning, "Texture file does not exist " + fullPath);
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
        vtkF3DLog::Print(vtkF3DLog::Severity::Warning, "Cannot open texture file " + fullPath);
      }
    }
  }

  return texture;
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportLights(vtkRenderer* ren)
{
  ren->RemoveAllLights();
  ren->AutomaticLightCreationOff();

  if (!ren->GetUseImageBasedLighting())
  {
    vtkNew<vtkLightKit> lightKit;
    lightKit->AddLightsToRenderer(ren);
  }
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportProperties(vtkRenderer* vtkNotUsed(ren)) {}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::PrintSelf(std::ostream& os, vtkIndent indent)
{
  vtkImporter::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::SetFileName(const char* arg)
{
  this->TemporalInformationUpdated = false;
  this->Reader->SetFileName(std::string(arg));
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::SetOptions(const f3d::options* options)
{
  this->Options = options;
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::CanReadFile()
{
  return this->Reader->IsReaderValid();
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetOutputsDescription()
{
  return this->OutputDescription;
}

//----------------------------------------------------------------------------
std::string vtkF3DGenericImporter::GetMultiBlockDescription(
  vtkMultiBlockDataSet* mb, vtkIndent indent)
{
  std::stringstream ss;
  for (vtkIdType i = 0; i < mb->GetNumberOfBlocks(); i++)
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
void vtkF3DGenericImporter::UpdateTimeStep(double timestep)
{
  this->PostPro->UpdateTimeStep(timestep);
}
