#include "vtkF3DRendererWithColoring.h"

#include "f3d_options.h"

#include <vtkColorTransferFunction.h>
#include <vtkDataSetAttributes.h>
#include <vtkFieldData.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPolyData.h>
#include <vtkVolumeProperty.h>

vtkStandardNewMacro(vtkF3DRendererWithColoring);

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::Initialize(
  const f3d::options& options, const std::string& fileInfo)
{
  this->Superclass::Initialize(options, fileInfo);

  this->SetScalarBarActor(nullptr);
  this->SetGeometryActor(nullptr);
  this->SetPointSpritesActor(nullptr);
  this->SetVolumeProp(nullptr);
  this->SetPolyDataMapper(nullptr);
  this->SetPointGaussianMapper(nullptr);
  this->SetVolumeMapper(nullptr);

  options.get("point-sprites", this->UsePointSprites);
  options.get("volume", this->UseVolume);
  options.get("inverse", this->UseInverseOpacityFunction);
  options.get("bar", this->ScalarBarVisible);

  options.get("range", this->SpecifiedRange);
  options.get("colormap", this->Colormap);

  this->PointDataForColoring = nullptr;
  this->CellDataForColoring = nullptr;
  this->DataForColoring = nullptr;
  this->ArrayIndexForColoring = -1;
  this->ComponentForColoring = -1;

  this->ColorTransferFunctionConfigured = false;
  this->PolyDataMapperConfigured = false;
  this->PointGaussianMapperConfigured = false;
  this->VolumeConfigured = false;
  this->ScalarBarActorConfigured = false;

  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::GenerateMetaDataDescription()
{
  std::string description;
  description += " \n";
  if (this->PolyDataMapper)
  {
    vtkDataSet* dataset = this->PolyDataMapper->GetInput();
    if (dataset)
    {
      description += " Number of points: ";
      description += std::to_string(dataset->GetNumberOfPoints());
      description += " \n Number of cells: ";
      description += std::to_string(dataset->GetNumberOfCells());
      description += " \n";

      // Field Data
      vtkFieldData* fieldData = dataset->GetFieldData();
      int nbArrays = fieldData->GetNumberOfArrays();
      for (vtkIdType i = 0; i < nbArrays; i++)
      {
        vtkAbstractArray* array = fieldData->GetAbstractArray(i);
        if (array)
        {
          vtkIdType nbTuples = array->GetNumberOfTuples();
          if (nbTuples == 1)
          {
            description += " ";
            description += array->GetName();
            description += " = ";
            description += array->GetVariantValue(0).ToString();
            description += " \n";
          }
        }
      }
    }
    else
    {
      description += " Unavailable\n";
    }
  }
  else
  {
    description += " Unavailable\n";
  }

  return description;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetUsePointSprites(bool use)
{
  this->UsePointSprites = use;
  this->UpdateInternalActors();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRendererWithColoring::UsingPointSprites()
{
  return this->UsePointSprites;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetUseVolume(bool use)
{
  this->UseVolume = use;
  this->UpdateInternalActors();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRendererWithColoring::UsingVolume()
{
  return this->UseVolume;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetUseInverseOpacityFunction(bool use)
{
  this->UseInverseOpacityFunction = use;
  if (this->VolumeProp)
  {
    vtkPiecewiseFunction* pwf = this->VolumeProp->GetProperty()->GetScalarOpacity();
    if (pwf->GetSize() == 2)
    {
      double range[2];
      pwf->GetRange(range);

      pwf->RemoveAllPoints();
      pwf->AddPoint(range[0], this->UseInverseOpacityFunction ? 1.0 : 0.0);
      pwf->AddPoint(range[1], this->UseInverseOpacityFunction ? 0.0 : 1.0);
    }
    this->SetupRenderPasses();
  }
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRendererWithColoring::UsingInverseOpacityFunction()
{
  return this->UseInverseOpacityFunction;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleArrayIndexForColoring()
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
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleArrayForColoring(bool checkCurrent)
{
  if (this->DataForColoring)
  {
    for (int i = 0; i < this->DataForColoring->GetNumberOfArrays(); i++)
    {
      if (checkCurrent)
      {
        // Decrement before incrementing to keep this loop as simple as possible
        this->ArrayIndexForColoring--;
      }
      this->CycleArrayIndexForColoring();
      this->ArrayForColoring = this->DataForColoring->GetArray(this->ArrayIndexForColoring);
      if (this->ArrayForColoring || this->ArrayIndexForColoring == -1)
      {
        this->CheckCurrentComponentForColoring();
        break;
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CheckCurrentComponentForColoring()
{
  if (this->ArrayForColoring)
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
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleComponentForColoring()
{
  if (this->ArrayForColoring)
  {
    if (this->ArrayForColoring->GetNumberOfComponents() > 4)
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
        (this->ComponentForColoring + 3) % (this->ArrayForColoring->GetNumberOfComponents() + 2) -
        2;
    }
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleFieldForColoring()
{
  // A generic approach will be better when adding categorical field data coloring
  if (this->DataForColoring == this->PointDataForColoring)
  {
    this->DataForColoring = this->CellDataForColoring;
  }
  else // if (this->DataForColoring == this->CellDataForColoring)
  {
    this->DataForColoring = this->PointDataForColoring;
  }
  // Check current index is a valid coloring index
  // If not, find another one
  this->CycleArrayForColoring(true);
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::PrintColoringInfo()
{
/*  if (this->ArrayForColoring)
  {
    F3DLog::Print(F3DLog::Severity::Info, "Coloring using ",
      (this->DataForColoring == this->PointDataForColoring ? "point" : "cell"), " array named ",
      this->ArrayForColoring->GetName(), ", ",
      vtkF3DRendererWithColoring::ComponentToString(this->ComponentForColoring), ".");
  }
  else
  {
    F3DLog::Print(F3DLog::Severity::Info, "Not coloring");
  }*/ // TODO Rework this output
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleScalars(int cycleType)
{
  switch (cycleType)
  {
    case (F3D_FIELD_CYCLE):
      this->CycleFieldForColoring();
      break;
    case (F3D_ARRAY_CYCLE):
      this->CycleArrayForColoring();
      break;
    case (F3D_COMPONENT_CYCLE):
      this->CycleComponentForColoring();
      break;
    default:
      break;
  }

  this->ColorTransferFunctionConfigured = false;
  this->PolyDataMapperConfigured = false;
  this->PointGaussianMapperConfigured = false;
  this->VolumeConfigured = false;
  this->ScalarBarActorConfigured = false;

  this->UpdateInternalActors();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ShowScalarBar(bool show)
{
  this->ScalarBarVisible = show;
  this->UpdateScalarBarVisibility();
  this->SetupRenderPasses();
  this->CheatSheetNeedUpdate = true;
}

//----------------------------------------------------------------------------
bool vtkF3DRendererWithColoring::IsScalarBarVisible()
{
  return this->ScalarBarVisible;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::FillCheatSheetHotkeys(std::stringstream& cheatSheetText)
{
  if (this->DataForColoring)
  {
    cheatSheetText << " C: Cell scalars coloring ["
                   << (this->DataForColoring == this->CellDataForColoring ? "ON" : "OFF") << "]\n";
    cheatSheetText << " S: Scalars coloring ["
                   << (this->ArrayForColoring ? vtkF3DRendererWithColoring::ShortName(
                                                  this->ArrayForColoring->GetName(), 19)
                                              : "OFF")
                   << "]\n";
    cheatSheetText << " Y: Coloring compponent ["
                   << vtkF3DRendererWithColoring::ComponentToString(this->ComponentForColoring)
                   << "]\n";
    cheatSheetText << " B: Scalar bar " << (this->ScalarBarVisible ? "[ON]" : "[OFF]") << "\n";
  }

  if (this->VolumeProp)
  {
    cheatSheetText << " V: Volume representation " << (this->UseVolume ? "[ON]" : "[OFF]") << "\n";
    cheatSheetText << " I: Inverse volume opacity "
                   << (this->UseInverseOpacityFunction ? "[ON]" : "[OFF]") << "\n";
  }

  if (this->PointGaussianMapper)
  {
    cheatSheetText << " O: Point sprites " << (this->UsePointSprites ? "[ON]" : "[OFF]") << "\n";
  }
  this->Superclass::FillCheatSheetHotkeys(cheatSheetText);
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::UpdateInternalActors()
{
  // Make sure ArrayForColoring is pointing to the right array
  if (this->DataForColoring)
  {
    this->ArrayForColoring = this->DataForColoring->GetArray(this->ArrayIndexForColoring);
  }

  if (this->Verbose)
  {
    this->PrintColoringInfo();
  }

  bool volumeVisibility = !this->UseRaytracing && this->UseVolume;
  if (this->ArrayForColoring || volumeVisibility)
  {
    // When showing volume, always try to find an array to color with
    if (volumeVisibility && !this->ArrayForColoring)
    {
      this->CycleArrayForColoring();
    }

    if (!this->ArrayForColoring)
    {
      vtkWarningMacro("No array to color with");
    }
    if (!this->ColorTransferFunctionConfigured)
    {
      this->ConfigureRangeAndCTFForColoring(this->ArrayForColoring, this->ComponentForColoring);
      this->ColorTransferFunctionConfigured = true;
    }
  }

  if (this->GeometryActor)
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
  }
  if (this->PointSpritesActor)
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
  }
  if (this->VolumeProp)
  {
    vtkSmartVolumeMapper* mapper =
      vtkSmartVolumeMapper::SafeDownCast(this->VolumeProp->GetMapper());
    if (volumeVisibility && (!mapper || !mapper->GetInput() || !this->ArrayForColoring))
    {
      vtkWarningMacro("Cannot use volume with this dataset or with the requested array");
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
  }
  this->UpdateScalarBarVisibility();
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::UpdateScalarBarVisibility()
{
  if (this->ScalarBarActor)
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
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetColoring(vtkDataSetAttributes* pointData,
  vtkDataSetAttributes* cellData, bool useCellData, int arrayIndex, int component)
{
  this->ArrayForColoring = nullptr;
  this->PointDataForColoring = pointData;
  this->CellDataForColoring = cellData;
  this->DataForColoring = useCellData ? this->CellDataForColoring : this->PointDataForColoring;
  this->ArrayIndexForColoring = arrayIndex;
  this->ComponentForColoring = component;
  if (!this->DataForColoring ||
    this->ArrayIndexForColoring >= this->DataForColoring->GetNumberOfArrays() ||
    this->ArrayIndexForColoring < -1)
  {
    vtkWarningMacro("Invalid coloring values");
    this->ArrayIndexForColoring = -1;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureVolumeForColoring(vtkSmartVolumeMapper* mapper,
  vtkVolume* volume, vtkDataArray* array, int component, vtkColorTransferFunction* ctf,
  double range[2], bool cellFlag, bool inverseOpacityFlag)
{
  if (!array)
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
      // comp > 4 is actually not supported and would fail with an error
      vtkWarningWithObjectMacro(nullptr,
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
void vtkF3DRendererWithColoring::ConfigureMapperForColoring(vtkPolyDataMapper* mapper,
  vtkDataArray* array, int component, vtkColorTransferFunction* ctf, double range[2], bool cellFlag)
{
  if (!array)
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
      // comp > 4 is actually not supported and would fail with an error
      vtkWarningWithObjectMacro(nullptr,
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
void vtkF3DRendererWithColoring::ConfigureRangeAndCTFForColoring(vtkDataArray* array, int component)
{
  if (!array || component == -2)
  {
    return;
  }

  if (component >= array->GetNumberOfComponents())
  {
    vtkErrorMacro(<< "Invalid component index: " << component);
    return;
  }

  // Get range
  if (this->SpecifiedRange.size() == 2)
  {
    this->ColorRange[0] = this->SpecifiedRange[0];
    this->ColorRange[1] = this->SpecifiedRange[1];
  }
  else
  {
    if (this->SpecifiedRange.size() > 0)
    {
      vtkWarningMacro(
        "The range specified does not have exactly 2 values, using automatic range.");
    }
    array->GetRange(this->ColorRange, component);
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
      vtkWarningMacro(
        "Specified color map list count is not a multiple of 4, ignoring it.");
    }
  }

  if (component >= 0)
  {
    this->ColorTransferFunction->SetVectorModeToComponent();
    this->ColorTransferFunction->SetVectorComponent(component);
  }
  else
  {
    this->ColorTransferFunction->SetVectorModeToMagnitude();
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureScalarBarActorForColoring(
  vtkScalarBarActor* scalarBar, vtkDataArray* array, int component, vtkColorTransferFunction* ctf)
{
  if (!array)
  {
    return;
  }

  std::string title = array->GetName();
  title += " (";
  title += vtkF3DRendererWithColoring::ComponentToString(component);
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
std::string vtkF3DRendererWithColoring::ComponentToString(int component)
{
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
    std::string ret = "Component #";
    ret += std::to_string(component);
    return ret;
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
