#include "vtkF3DRendererWithColoring.h"

#include "F3DLog.h"
#include "vtkF3DConfigure.h"
#include "vtkF3DGenericImporter.h"

#include <vtkBoundingBox.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkFieldData.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>

#include <sstream>

vtkStandardNewMacro(vtkF3DRendererWithColoring);
vtkCxxSetObjectMacro(vtkF3DRendererWithColoring, Importer, vtkF3DGenericImporter);

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::Initialize(const std::string& up)
{
  this->Superclass::Initialize(up);

/*  this->SetScalarBarActor(nullptr);
  this->SetGeometryActor(nullptr);
  this->SetPointSpritesActor(nullptr);
  this->SetVolumeProp(nullptr);
  this->SetPolyDataMapper(nullptr);
  this->SetPointGaussianMapper(nullptr);
  this->SetVolumeMapper(nullptr);*/

/*  this->PointDataForColoring = nullptr;
  this->CellDataForColoring = nullptr;
  this->DataForColoring = nullptr;*/
  this->ArrayIndexForColoring = -1;
  this->ComponentForColoring = -1;

  this->AddActor2D(this->ScalarBarActor);
  this->ScalarBarActor->SetVisibility(false);

  this->ColorTransferFunctionConfigured = false;
  this->PolyDataMapperConfigured = false;
  this->PointGaussianMapperConfigured = false;
  this->VolumeConfigured = false;
  this->ScalarBarActorConfigured = false;

  this->CheatSheetNeedUpdate = true;

  this->ColoringTimeStamp.Modified();
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetPointSize(double pointSize)
{
  this->Superclass::SetPointSize(pointSize);

  if (!this->Importer)
  {
    return;
  }

  const vtkBoundingBox& bbox = this->Importer->GetGeometryBoundingBox();
  double gaussianPointSize = 1.0;
  if (bbox.IsValid())
  {
    gaussianPointSize = pointSize * bbox.GetDiagonalLength() * 0.001;
  }

  auto psActorsAndMappers = this->Importer->GetPointSpritesActorsAndMappers();
  for (auto psActorAndMapper : psActorsAndMappers)
  {
    psActorAndMapper.second->SetScaleFactor(gaussianPointSize);
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ShowScalarBar(bool show)
{
  if (this->ScalarBarVisible != show)
  {
    this->ScalarBarVisible = show;
    this->CheatSheetNeedUpdate = true;
    this->ColoringTimeStamp.Modified();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetUsePointSprites(bool use)
{
  if (this->UsePointSprites != use)
  {
    this->UsePointSprites = use;
    this->CheatSheetNeedUpdate = true;
    this->ColoringTimeStamp.Modified();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetUseVolume(bool use)
{
  if (this->UseVolume != use)
  {
    this->UseVolume = use;
    this->CheatSheetNeedUpdate = true;
    this->ColoringTimeStamp.Modified();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetUseInverseOpacityFunction(bool use)
{
  if (this->UseInverseOpacityFunction != use)
  {
    this->UseInverseOpacityFunction = use;

    auto volPropsAndMappers = this->Importer->GetVolumePropsAndMappers();
    for (auto volPropAndMapper : volPropsAndMappers)
    {
      if(volPropAndMapper.first)
      {
        vtkPiecewiseFunction* pwf = volPropAndMapper.first->GetProperty()->GetScalarOpacity();
        if (pwf->GetSize() == 2)
        {
          double range[2];
          pwf->GetRange(range);

          pwf->RemoveAllPoints();
          pwf->AddPoint(range[0], this->UseInverseOpacityFunction ? 1.0 : 0.0);
          pwf->AddPoint(range[1], this->UseInverseOpacityFunction ? 0.0 : 1.0);
        }
      }
    }
    this->VolumeConfigured = false;
    this->CheatSheetNeedUpdate = true;
    this->ColoringTimeStamp.Modified();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetScalarBarRange(const std::vector<double>& range)
{
  if (this->UserScalarBarRange != range)
  {
    this->UserScalarBarRange = range;
    this->ColorTransferFunctionConfigured = false;
    this->PolyDataMapperConfigured = false;
    this->PointGaussianMapperConfigured = false;
    this->VolumeConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->ColoringTimeStamp.Modified();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetColormap(const std::vector<double>& colormap)
{
  if (this->Colormap != colormap)
  {
    this->Colormap = colormap;

    this->ColorTransferFunctionConfigured = false;
    this->PolyDataMapperConfigured = false;
    this->PointGaussianMapperConfigured = false;
    this->VolumeConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->ColoringTimeStamp.Modified();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleScalars(CycleType type)
{
  switch (type)
  {
    case (CycleType::F3D_CYCLE_NONE):
      return;
      break;
    case (CycleType::F3D_CYCLE_FIELD):
      this->CycleFieldForColoring();
      break;
    case (CycleType::F3D_CYCLE_ARRAY_INDEX):
      this->CycleArrayIndexForColoring();
      break;
    case (CycleType::F3D_CYCLE_COMPONENT):
      this->CycleComponentForColoring();
      break;
    default:
      break;
  }

  // Check attributes are valid and cycle recursively if needed
  this->CycleScalars(this->CheckColoring());

  this->ColorTransferFunctionConfigured = false;
  this->PolyDataMapperConfigured = false;
  this->PointGaussianMapperConfigured = false;
  this->VolumeConfigured = false;
  this->ScalarBarActorConfigured = false;
  this->CheatSheetNeedUpdate = true;

  this->ColoringTimeStamp.Modified();
}

//----------------------------------------------------------------------------
vtkF3DRendererWithColoring::CycleType vtkF3DRendererWithColoring::CheckColoring()
{
  // Never force change of anything if we are currently not coloring
  if (this->ArrayIndexForColoring == -1)
  {
    return CycleType::F3D_CYCLE_NONE;
  }

  // Never force change of CellData/PointData coloring on the user
  if (this->Importer->GetNumberOfIndexesForColoring(this->UseCellColoring) == 0)
  {
    return CycleType::F3D_CYCLE_NONE;
  }

  // Change the array index only if current is not valid and user just change the field type
  vtkF3DGenericImporter::ColoringInfo info;
  if (!this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    return CycleType::F3D_CYCLE_ARRAY_INDEX;
  }

  // Always change the component if invalid
  if (this->ComponentForColoring >= info.MaximumNumberOfComponents)
  {
    return CycleType::F3D_CYCLE_COMPONENT;
  }

  return CycleType::F3D_CYCLE_NONE;
}

//----------------------------------------------------------------------------
/*void vtkF3DRendererWithColoring::SetColoringAttributes(
  vtkDataSetAttributes* pointData, vtkDataSetAttributes* cellData)
{
  if (this->PointDataForColoring != pointData || this->CellDataForColoring != cellData)
  {
    this->PointDataForColoring = pointData;
    this->CellDataForColoring = cellData;

    this->DataForColoring = nullptr;
    this->ArrayIndexForColoring = -1;
    this->ComponentForColoring = -1;
    this->ArrayForColoring = nullptr;

    this->ColorTransferFunctionConfigured = false;
    this->PolyDataMapperConfigured = false;
    this->PointGaussianMapperConfigured = false;
    this->VolumeConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->ColoringTimeStamp.Modified();
  }
}*/

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetColoring(
  bool useCellData, const std::string& arrayName, int component)
{
  if (!this->Importer)
  {
    return;
  }

  if (this->GetColoringUseCell() != useCellData ||
    this->GetColoringArrayName() != arrayName ||
    this->GetColoringComponent() != component)
  {
    this->UseCellColoring = useCellData;

    if (arrayName.empty())
    {
      this->ArrayIndexForColoring = this->Importer->GetNumberOfIndexesForColoring(this->UseCellColoring) > 0 ? 0 : -1;
    }
    else
    {
      this->ArrayIndexForColoring = this->Importer->FindIndexForColoring(useCellData, arrayName);
      //    this->ArrayForColoring = this->DataForColoring->GetArray(this->ArrayIndexForColoring);
    }

    this->ComponentForColoring = component;

    this->ColorTransferFunctionConfigured = false;
    this->PolyDataMapperConfigured = false;
    this->PointGaussianMapperConfigured = false;
    this->VolumeConfigured = false;
    this->ScalarBarActorConfigured = false;
    this->ColoringTimeStamp.Modified();
  }
}

//----------------------------------------------------------------------------
bool vtkF3DRendererWithColoring::GetColoringUseCell()
{
  return this->UseCellColoring;
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::GetColoringArrayName()
{
  if (!this->Importer)
  {
    return F3D_RESERVED_STRING;
  }

  vtkF3DGenericImporter::ColoringInfo info;
  if (this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    return info.Name;
  }
  else
  {
    return F3D_RESERVED_STRING;
  }
}

//----------------------------------------------------------------------------
int vtkF3DRendererWithColoring::GetColoringComponent()
{
  return this->ComponentForColoring;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::UpdateColoringActors()
{
  if (!this->Importer)
  {
    return;
  }

  // Early return if nothing changed
  if (this->ColoringUpdateTime >= this->ColoringTimeStamp.GetMTime())
  {
    return;
  }

  this->ColoringUpdateTime = this->ColoringTimeStamp.GetMTime();

  // Recover coloring information
  vtkF3DGenericImporter::ColoringInfo info;
  bool hasColoring = this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info);

  bool volumeVisible = !this->UseRaytracing && this->UseVolume;
  if (!hasColoring && volumeVisible)
  {
    // When showing volume, always try to find an array to color with
    this->CycleScalars(vtkF3DRendererWithColoring::CycleType::F3D_CYCLE_ARRAY_INDEX);
    hasColoring = this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info);
  }

  if (hasColoring && !this->ColorTransferFunctionConfigured)
  {
    this->ConfigureRangeAndCTFForColoring(info);
    this->ColorTransferFunctionConfigured = true; // TODO is this still working ?
  }

/*  bool volumeVisibility = !this->UseRaytracing && this->UseVolume;
  if (this->ArrayForColoring || volumeVisibility)
  {
    // When showing volume, always try to find an array to color with
    if (volumeVisibility && !this->ArrayForColoring)
    {
      this->CycleArrayForColoring();
    }

    if (!this->ArrayForColoring)
    {
      F3DLog::Print(F3DLog::Severity::Warning, "No array to color with");
    }
    if (!this->ColorTransferFunctionConfigured)
    {
      this->ConfigureRangeAndCTFForColoring(this->ArrayForColoring, this->ComponentForColoring);
      this->ColorTransferFunctionConfigured = true;
    }
  }*/

/*  if (this->GeometryActor)
  {
    bool visible = this->UseRaytracing || (!this->UseVolume && !this->UsePointSprites);
    this->GeometryActor->SetVisibility(visible);
    if (this->PolyDataMapper)
    {
      if (visible && this->ArrayForColoring)
      {
        if (!this->PolyDataMapperConfigured)
        {
          vtkF3DRendererWithColoring::ConfigureMapperForColoring(this->PolyDataMapper,
            this->ArrayForColoring, this->ComponentForColoring, this->ColorTransferFunction,
            this->ColorRange, this->DataForColoring == this->CellDataForColoring);
          this->PolyDataMapperConfigured = true;
        }
        this->PolyDataMapper->ScalarVisibilityOn();
      }
      else
      {
        this->PolyDataMapper->ScalarVisibilityOff();
      }
    }
  }*/

/*  if (this->PointSpritesActor)
  {
    bool visible = !this->UseRaytracing && !this->UseVolume && this->UsePointSprites;
    this->PointSpritesActor->SetVisibility(visible);
    if (this->PointGaussianMapper)
    {
      if (visible && this->ArrayForColoring)
      {
        if (!this->PointGaussianMapperConfigured)
        {
          vtkF3DRendererWithColoring::ConfigureMapperForColoring(this->PointGaussianMapper,
            this->ArrayForColoring, this->ComponentForColoring, this->ColorTransferFunction,
            this->ColorRange, this->DataForColoring == this->CellDataForColoring);
          this->PointGaussianMapperConfigured = true;
        }
        this->PointGaussianMapper->ScalarVisibilityOn();
      }
      else
      {
        this->PointGaussianMapper->ScalarVisibilityOff();
      }
    }
  }*/
/*  if (this->VolumeProp)
  {
    vtkSmartVolumeMapper* mapper =
      vtkSmartVolumeMapper::SafeDownCast(this->VolumeProp->GetMapper());
    if (volumeVisibility && (!mapper || !mapper->GetInput() || !this->ArrayForColoring))
    {
      F3DLog::Print(
        F3DLog::Severity::Error, "Cannot use volume with this dataset or with the requested array");
      volumeVisibility = false;
    }
    if (volumeVisibility && this->VolumeMapper && this->VolumeMapper->GetInput() &&
      !this->VolumeConfigured)
    {
      vtkF3DRendererWithColoring::ConfigureVolumeForColoring(this->VolumeMapper, this->VolumeProp,
        this->ArrayForColoring, this->ComponentForColoring, this->ColorTransferFunction,
        this->ColorRange, this->DataForColoring == this->CellDataForColoring,
        this->UseInverseOpacityFunction);
      this->VolumeConfigured = true;
    }
    this->VolumeProp->SetVisibility(volumeVisibility);
  }*/

  // Handle surface geometry
  bool geometriesVisible = this->UseRaytracing || (!this->UseVolume && !this->UsePointSprites);
  auto actorsAndMappers = this->Importer->GetGeometryActorsAndMappers();
  for (size_t i = 0; i < actorsAndMappers.size(); i++)
  {
    auto actorAndMapper = actorsAndMappers[i];
    vtkDataArray* coloringArray = nullptr;
    if (hasColoring && info.Arrays.size() > i)
    {
      coloringArray = info.Arrays[i];
    }
    actorAndMapper.first->SetVisibility(geometriesVisible);
    if (geometriesVisible && coloringArray)
    {
//      if (!this->PolyDataMapperConfigured) // TODO should we skip this ?
      {
        vtkF3DRendererWithColoring::ConfigureMapperForColoring(actorAndMapper.second,
          coloringArray, this->ComponentForColoring, this->ColorTransferFunction,
          this->ColorRange, this->UseCellColoring);
//        this->PolyDataMapperConfigured = true;
      }
      actorAndMapper.second->ScalarVisibilityOn();
    }
    else
    {
      actorAndMapper.second->ScalarVisibilityOff();
    }
  }

  // Handle point sprites
  bool pointSpritesVisible = !this->UseRaytracing && !this->UseVolume && this->UsePointSprites;
  auto psActorsAndMappers = this->Importer->GetPointSpritesActorsAndMappers();
  for (size_t i = 0; i < psActorsAndMappers.size(); i++)
  {
    auto actorAndMapper = psActorsAndMappers[i];
    vtkDataArray* coloringArray = nullptr;
    if (hasColoring && info.Arrays.size() > i)
    {
      coloringArray = info.Arrays[i];
    }
    actorAndMapper.first->SetVisibility(pointSpritesVisible);
    if (pointSpritesVisible && coloringArray)
    {
//      if (!this->PointGaussianMapperConfigured) // TODO should we skip this ?
      {
        vtkF3DRendererWithColoring::ConfigureMapperForColoring(actorAndMapper.second,
          coloringArray, this->ComponentForColoring, this->ColorTransferFunction,
          this->ColorRange, this->UseCellColoring);
//        this->PointGaussianMapperConfigured = true;
      }
      actorAndMapper.second->ScalarVisibilityOn();
    }
    else
    {
      actorAndMapper.second->ScalarVisibilityOff();
    }
  }

  // Handle Volume prop
  auto volPropsAndMappers = this->Importer->GetVolumePropsAndMappers();
  for (size_t i = 0; i < volPropsAndMappers.size(); i++)
  {
    auto propAndMapper = volPropsAndMappers[i];
    vtkDataArray* coloringArray = nullptr;
    if (hasColoring && info.Arrays.size() > i)
    {
      coloringArray = info.Arrays[i];
    }
    if (!volumeVisible)
    {
      propAndMapper.first->SetVisibility(false);
    }
    else if (volumeVisible && !coloringArray)
    {
      F3DLog::Print(
        F3DLog::Severity::Error, "Cannot use volume with this dataset or with the requested array");
      propAndMapper.first->SetVisibility(false);
    }
    else
    {
      vtkF3DRendererWithColoring::ConfigureVolumeForColoring(propAndMapper.second, propAndMapper.first,
        coloringArray, this->ComponentForColoring, this->ColorTransferFunction,
        this->ColorRange, this->UseCellColoring,
        this->UseInverseOpacityFunction);
//      this->VolumeConfigured = true; TODO does it matter ?
      propAndMapper.first->SetVisibility(true);
    }
  }

  //this->UpdateScalarBarVisibility();

  // Handle scalar bar
  bool barVisible =
    this->ScalarBarVisible && hasColoring && this->ComponentForColoring >= -1;
  this->ScalarBarActor->SetVisibility(barVisible);
  if (barVisible && !this->ScalarBarActorConfigured)
  {
    vtkF3DRendererWithColoring::ConfigureScalarBarActorForColoring(this->ScalarBarActor,
      info.Name, this->ComponentForColoring, this->ColorTransferFunction);
    this->ScalarBarActorConfigured = true; // TODO do we keep it ?
  }

  this->SetupRenderPasses();
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::GetColoringDescription()
{
  if (!this->Importer)
  {
    return "";
  }

  std::stringstream stream;
  vtkF3DGenericImporter::ColoringInfo info;
  if (this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    stream << "Coloring using "
           << (this->UseCellColoring ? "point" : "cell")
           << " array named " << info.Name << ", "
           << vtkF3DRendererWithColoring::ComponentToString(this->ComponentForColoring) << "\n";
  }
  else
  {
    stream << "Not coloring\n";
  }
  return stream.str();
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureMapperForColoring(vtkPolyDataMapper* mapper,
  vtkDataArray* array, int component, vtkColorTransferFunction* ctf, double range[2], bool cellFlag)
{
  if (!array || component >= array->GetNumberOfComponents())
  {
    return;
  }

  mapper->SetColorModeToMapScalars();
  mapper->SelectColorArray(array->GetName());
  mapper->SetScalarMode(
    cellFlag ? VTK_SCALAR_MODE_USE_CELL_FIELD_DATA : VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
  mapper->ScalarVisibilityOn();

  if (component == -2)
  {
    if (array->GetNumberOfComponents() > 4)
    {
      // comp > 4 is actually not supported and would fail with a vtk error
      F3DLog::Print(F3DLog::Severity::Warning,
        "Direct scalars rendering not supported by array with more than 4 components");
    }
    else
    {
      mapper->SetColorModeToDirectScalars();
    }
  }
  else
  {
    mapper->SetColorModeToMapScalars();
    mapper->SetScalarRange(range);
    mapper->SetLookupTable(ctf);
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureVolumeForColoring(vtkSmartVolumeMapper* mapper,
  vtkVolume* volume, vtkDataArray* array, int component, vtkColorTransferFunction* ctf,
  double range[2], bool cellFlag, bool inverseOpacityFlag)
{
  if (!array || component >= array->GetNumberOfComponents())
  {
    return;
  }

  mapper->SetScalarMode(
    cellFlag ? VTK_SCALAR_MODE_USE_CELL_FIELD_DATA : VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
  mapper->SelectScalarArray(array->GetName());

  if (component >= 0)
  {
    mapper->SetVectorMode(vtkSmartVolumeMapper::COMPONENT);
    mapper->SetVectorComponent(component);
  }
  else if (component == -1)
  {
    mapper->SetVectorMode(vtkSmartVolumeMapper::MAGNITUDE);
  }
  else if (component == -2)
  {
    if (array->GetNumberOfComponents() > 4)
    {
      // comp > 4 is actually not supported and would fail with a vtk error
      F3DLog::Print(F3DLog::Severity::Warning,
        "Direct scalars rendering not supported by array with more than 4 components");
    }
    else
    {
      mapper->SetVectorMode(vtkSmartVolumeMapper::DISABLED);
    }
  }

  vtkNew<vtkPiecewiseFunction> otf;
  otf->AddPoint(range[0], inverseOpacityFlag ? 1.0 : 0.0);
  otf->AddPoint(range[1], inverseOpacityFlag ? 0.0 : 1.0);

  vtkNew<vtkVolumeProperty> property;
  property->SetColor(ctf);
  property->SetScalarOpacity(otf);
  property->ShadeOff();
  property->SetInterpolationTypeToLinear();

  volume->SetProperty(property);
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureScalarBarActorForColoring(
  vtkScalarBarActor* scalarBar, const std::string& arrayName, int component, vtkColorTransferFunction* ctf)
{
  std::string title = arrayName;
  title += " (";
  title += this->ComponentToString(component);
  title += ")";

  scalarBar->SetLookupTable(ctf);
  scalarBar->SetTitle(title.c_str());
  scalarBar->SetNumberOfLabels(4);
  scalarBar->SetOrientationToHorizontal();
  scalarBar->SetWidth(0.8);
  scalarBar->SetHeight(0.07);
  scalarBar->SetPosition(0.1, 0.01);
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureRangeAndCTFForColoring(const vtkF3DGenericImporter::ColoringInfo& info)
{
  if (this->ComponentForColoring == -2) // TODO
  {
    return;
  }

  // Set range
  if (this->UserScalarBarRange.size() == 2)
  {
    this->ColorRange[0] = this->UserScalarBarRange[0];
    this->ColorRange[1] = this->UserScalarBarRange[1];
  }
  else if (this->ComponentForColoring >= 0)
  {
    this->ColorRange[0] = info.ComponentRanges[this->ComponentForColoring][0];
    this->ColorRange[1] = info.ComponentRanges[this->ComponentForColoring][1];
  }
  else
  {
    this->ColorRange[0] = info.MagnitudeRange[0];
    this->ColorRange[1] = info.MagnitudeRange[1];
  }

  // Create lookup table
  this->ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  if (this->Colormap.size() > 0)
  {
    if (this->Colormap.size() % 4 == 0)
    {
      for (size_t i = 0; i < this->Colormap.size(); i += 4)
      {
        double val = this->Colormap[i];
        double r = this->Colormap[i + 1];
        double g = this->Colormap[i + 2];
        double b = this->Colormap[i + 3];
        this->ColorTransferFunction->AddRGBPoint(
          this->ColorRange[0] + val * (this->ColorRange[1] - this->ColorRange[0]), r, g, b);
      }
    }
    else
    {
      F3DLog::Print(F3DLog::Severity::Warning,
        "Specified color map list count is not a multiple of 4, ignoring it.");
    }
  }

  if (this->ComponentForColoring >= 0)
  {
    this->ColorTransferFunction->SetVectorModeToComponent();
    this->ColorTransferFunction->SetVectorComponent(this->ComponentForColoring);
  }
  else
  {
    this->ColorTransferFunction->SetVectorModeToMagnitude();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::UpdateScalarBarVisibility()
{
/*  if (this->ScalarBarActor)
  {
    bool visible =
      this->ScalarBarVisible && this->ArrayForColoring && this->ComponentForColoring >= -1;
    this->ScalarBarActor->SetVisibility(visible);

    if (visible && !this->ScalarBarActorConfigured)
    {
      vtkF3DRendererWithColoring::ConfigureScalarBarActorForColoring(this->ScalarBarActor,
        this->ArrayForColoring, this->ComponentForColoring, this->ColorTransferFunction);
      this->ScalarBarActorConfigured = true;
    }
  }*/ // TODO
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::FillCheatSheetHotkeys(std::stringstream& cheatSheetText)
{
  if (!this->Importer)
  {
    return;
  }

  vtkF3DGenericImporter::ColoringInfo info;
  bool hasColoring = this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info);

  cheatSheetText << " C: Cell scalars coloring ["
                 << (this->UseCellColoring ? "ON" : "OFF") << "]\n";
  cheatSheetText << " S: Scalars coloring ["
                 << (hasColoring ? vtkF3DRendererWithColoring::ShortName( info.Name, 19) : "OFF")
                 << "]\n";
  cheatSheetText << " Y: Coloring component ["
                 << vtkF3DRendererWithColoring::ComponentToString(this->ComponentForColoring)
                 << "]\n";
  cheatSheetText << " B: Scalar bar " << (this->ScalarBarVisible ? "[ON]" : "[OFF]") << "\n";

  cheatSheetText << " V: Volume representation " << (this->UseVolume ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " I: Inverse volume opacity "
    << (this->UseInverseOpacityFunction ? "[ON]" : "[OFF]") << "\n";
  cheatSheetText << " O: Point sprites " << (this->UsePointSprites ? "[ON]" : "[OFF]") << "\n";
  this->Superclass::FillCheatSheetHotkeys(cheatSheetText);
}

//----------------------------------------------------------------------------
/*int vtkF3DRendererWithColoring::FindArrayIndexForColoring(
  vtkDataSetAttributes* dataForColoring, const std::string& arrayName)
{
  int index = -1;
  if (dataForColoring)
  {
    std::string usedArray = arrayName;

    // scalars == "", color with the first found array
    if (usedArray.empty())
    {
      vtkDataArray* array = dataForColoring->GetScalars();
      if (array)
      {
        const char* localArrayName = array->GetName();
        if (localArrayName)
        {
          // store the name for find the index below
          usedArray = localArrayName;
        }
      }
      else
      {
        for (int i = 0; i < dataForColoring->GetNumberOfArrays(); i++)
        {
          array = dataForColoring->GetArray(i);
          if (array)
          {
            index = i;
            break;
          }
        }
      }
    }

    if (index == -1)
    {
      // index not set yet, find it using the name if possible
      dataForColoring->GetAbstractArray(usedArray.c_str(), index);
    }
    if (index == -1 && !usedArray.empty() && usedArray != F3D_RESERVED_STRING)
    {
      // index not set and name is not the one reserved for not coloring
      F3DLog::Print(F3DLog::Severity::Warning, "Unknown scalar array: " + usedArray + "\n");
    }
  }
  return index;
}*/

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleFieldForColoring()
{
  // A generic approach will be better when adding categorical field data coloring
  this->UseCellColoring = !this->UseCellColoring;

/*  if (this->DataForColoring == this->PointDataForColoring)
  {
    this->DataForColoring = this->CellDataForColoring;
  }
  else // if (this->DataForColoring == this->CellDataForColoring)
  {
    this->DataForColoring = this->PointDataForColoring;
  }*/
  // Check current index is a valid coloring index
  // If not, find another one
//  this->CycleArrayForColoring(true); TODO ?
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleArrayIndexForColoring()
{
  if (!this->Importer)
  {
    return;
  }

  int nIndex = this->Importer->GetNumberOfIndexesForColoring(this->UseCellColoring);
  if (nIndex <= 0)
  {
    return;
  }

  if (this->UseVolume)
  {
    this->ArrayIndexForColoring =
      (this->ArrayIndexForColoring + 1) % nIndex;
  }
  else
  {
    // Cycle through arrays looping back to -1
    // -1 0 1 2 -1 0 1 2 ...
    this->ArrayIndexForColoring =
      (this->ArrayIndexForColoring + 2) % (nIndex + 1) - 1;
  }
}

//----------------------------------------------------------------------------
/*void vtkF3DRendererWithColoring::CycleArrayIndexForColoring()
{
  if (this->UseVolume)
  {
    this->ArrayIndexForColoring =
      (this->ArrayIndexForColoring + 1) % this->DataForColoring->GetNumberOfArrays();
  }
  else
  {
    // Cycle through arrays looping back to -1
    // -1 0 1 2 -1 0 1 2 ...
    this->ArrayIndexForColoring =
      (this->ArrayIndexForColoring + 2) % (this->DataForColoring->GetNumberOfArrays() + 1) - 1;
  }
}*/

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleComponentForColoring()
{
/*  if (this->ArrayForColoring)
  {
    if (this->ArrayForColoring->GetNumberOfComponents() > 4) // TODO cant really handle this case, does it matter ?
    {
      // -1 0 1 2 3 4 5 6
      this->ComponentForColoring =
        (this->ComponentForColoring + 2) % (this->ArrayForColoring->GetNumberOfComponents() + 1) -
        1;
    }
    else
    {
      // -2 -1 0 1 2 3 4
      this->ComponentForColoring =
        (this->ComponentForColoring + 3) % (this->ArrayForColoring->GetNumberOfComponents() + 2) - // Rethink this
        2;
    }
  }*/

  vtkF3DGenericImporter::ColoringInfo info;
  if (!this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    return;
  }

  // -2 -1 0 1 2 3 4
  this->ComponentForColoring =
    (this->ComponentForColoring + 3) % (info.MaximumNumberOfComponents + 2) - 2; // TODO Rethink this
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CheckCurrentComponentForColoring()
{
/*  if (this->ArrayForColoring) // TODO cant be done the same way
  {
    // Check direct scalars
    if (this->ArrayForColoring->GetNumberOfComponents() > 4 && this->ComponentForColoring == -2)
    {
      this->ComponentForColoring = -1;
    }
    // Check number of comps
    else if (this->ComponentForColoring >= this->ArrayForColoring->GetNumberOfComponents())
    {
      this->ComponentForColoring = this->ArrayForColoring->GetNumberOfComponents() - 1;
    }
  }*/
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::GenerateMetaDataDescription()
{
  if (!this->Importer)
  {
    return "";
  }

  // XXX Padding should not be handled by manipulating string
  // but on the actor directly, but it is not supported by VTK yet.

  // add eol before/after the string
  std::string description = "\n" + this->Importer->GetMetaDataDescription() + "\n";
  size_t index = 0;
  while (true)
  {
     index = description.find("\n", index);
     if (index == std::string::npos)
     {
       break;
     }
     // Add spaces after/before eol
     description.insert(index + 1, " ");
     description.insert(index, " ");
     index += 3;
  }

  return description;
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::ComponentToString(int component)
{
  // TODO switch to options
  if (component == -2)
  {
    return "Direct Scalars";
  }
  else if (component == -1)
  {
    return "Magnitude";
  }
  else
  {
    vtkF3DGenericImporter::ColoringInfo info;
    if (!this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
    {
      return "";
    }
    if (component >= info.MaximumNumberOfComponents)
    {
      return "";
    }

    std::string componentName;
    if (component < static_cast<int>(info.ComponentNames.size()))
    {
      componentName = info.ComponentNames[component];
    }
    if (componentName.empty())
    {
      componentName = "Component #";
      componentName += std::to_string(component);
    }
    return componentName;
  }
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::ShortName(const std::string& name, int maxChar)
{
  if (name.size() <= static_cast<size_t>(maxChar) || maxChar <= 3)
  {
    return name;
  }
  else
  {
    return name.substr(0, maxChar - 3) + "...";
  }
}
