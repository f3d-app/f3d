#include "vtkF3DGenericImporter.h"

#include "F3DLog.h"

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
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

#include <sstream>

vtkStandardNewMacro(vtkF3DGenericImporter);

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::UpdateTemporalInformation()
{
  if (!this->Reader)
  {
    F3DLog::Print(F3DLog::Severity::Warning, "Reader is not valid\n");
    return;
  }
  this->Reader->UpdateInformation();
  vtkInformation* readerInfo = this->Reader->GetOutputInformation(0);

  this->NbTimeSteps = readerInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  this->TimeRange = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE());
  this->TimeSteps = readerInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
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
  // forward progress event
  vtkNew<vtkEventForwarderCommand> forwarder;
  forwarder->SetTarget(this);
  this->Reader->AddObserver(vtkCommand::ProgressEvent, forwarder);

  this->PostPro->SetInputConnection(this->Reader->GetOutputPort());
  this->PostPro->Update();

  this->OutputDescription =
    vtkF3DGenericImporter::GetDataObjectDescription(this->Reader->GetOutputDataObject(0));

  vtkPolyData* surface = vtkPolyData::SafeDownCast(this->PostPro->GetOutput());
  vtkImageData* image = vtkImageData::SafeDownCast(this->PostPro->GetOutput(2));

  // Configure volume mapper
  this->VolumeMapper->SetInputConnection(this->PostPro->GetOutputPort(2));
  this->VolumeMapper->SetRequestedRenderModeToGPU();

  // Configure polydata mapper
  this->PolyDataMapper->InterpolateScalarsBeforeMappingOn();
  this->PolyDataMapper->SetInputConnection(this->PostPro->GetOutputPort(0));

  // Configure Point Gaussian mapper
  this->PointGaussianMapper->SetInputConnection(this->PostPro->GetOutputPort(1));
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

  vtkDataSet* dataSet = vtkImageData::SafeDownCast(this->PostPro->GetInput())
    ? vtkDataSet::SafeDownCast(image)
    : vtkDataSet::SafeDownCast(surface);

  this->PointDataForColoring = vtkDataSetAttributes::SafeDownCast(dataSet->GetPointData());
  this->CellDataForColoring = vtkDataSetAttributes::SafeDownCast(dataSet->GetCellData());

  // configure props
  this->VolumeProp->SetMapper(this->VolumeMapper);

  this->GeometryActor->SetMapper(this->PolyDataMapper);
  this->GeometryActor->GetProperty()->SetInterpolationToPBR();

  this->GeometryActor->GetProperty()->SetColor(this->SurfaceColor);
  this->GeometryActor->GetProperty()->SetOpacity(this->Opacity);
  this->GeometryActor->GetProperty()->SetRoughness(this->Roughness);
  this->GeometryActor->GetProperty()->SetMetallic(this->Metallic);
  this->GeometryActor->GetProperty()->SetLineWidth(this->LineWidth);
  this->GeometryActor->GetProperty()->SetPointSize(this->PointSize);

  this->PointSpritesActor->SetMapper(this->PointGaussianMapper);
  this->PointSpritesActor->GetProperty()->SetColor(this->SurfaceColor);
  this->PointSpritesActor->GetProperty()->SetOpacity(this->Opacity);

  // Textures
  auto colorTex = this->GetTexture(this->TextureBaseColor, true);
  this->GeometryActor->GetProperty()->SetBaseColorTexture(colorTex);
  this->GeometryActor->GetProperty()->SetORMTexture(this->GetTexture(this->TextureMaterial));
  this->GeometryActor->GetProperty()->SetEmissiveTexture(
    this->GetTexture(this->TextureEmissive, true));
  this->GeometryActor->GetProperty()->SetEmissiveFactor(this->EmissiveFactor);
  this->GeometryActor->GetProperty()->SetNormalTexture(this->GetTexture(this->TextureNormal));
  this->GeometryActor->GetProperty()->SetNormalScale(this->NormalScale);

  // If the input texture is RGBA, flag the actor as translucent
  if (colorTex && colorTex->GetImageDataInput(0)->GetNumberOfScalarComponents() == 4)
  {
    this->GeometryActor->ForceTranslucentOn();
  }

  // add props
  ren->AddActor2D(this->ScalarBarActor);
  ren->AddActor(this->GeometryActor);
  ren->AddActor(this->PointSpritesActor);
  ren->AddVolume(this->VolumeProp);

  this->ScalarBarActor->SetVisibility(false);
  this->GeometryActor->SetVisibility(false);
  this->PointSpritesActor->SetVisibility(false);
  this->VolumeProp->SetVisibility(false);
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
void vtkF3DGenericImporter::SetInternalReader(vtkAlgorithm* reader)
{
  this->Reader = reader;
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkF3DGenericImporter::CanReadFile()
{
  return this->Reader != nullptr;
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
void vtkF3DGenericImporter::UpdateTimeStep(double timestep)
{
  this->PostPro->UpdateTimeStep(timestep);
}

//----------------------------------------------------------------------------
int vtkF3DGenericImporter::ImportBegin()
{
  if (this->Reader)
  {
    this->Reader->Update();
    return 1;
  }

  return 0;
}
