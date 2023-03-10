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
#include <vtkPointGaussianMapper.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>

#include <sstream>

vtkStandardNewMacro(vtkF3DRendererWithColoring);

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetImporter(vtkF3DGenericImporter* importer)
{
  this->Importer = importer;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::Initialize(const std::string& up)
{
  this->Superclass::Initialize(up);

  this->ArrayIndexForColoring = -1;
  this->ComponentForColoring = -1;

  this->AddActor2D(this->ScalarBarActor);
  this->ScalarBarActor->VisibilityOff();

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

  if (this->Importer)
  {
    const vtkBoundingBox& bbox = this->Importer->GetGeometryBoundingBox();
    double gaussianPointSize = 1.0;
    if (bbox.IsValid())
    {
      gaussianPointSize = pointSize * bbox.GetDiagonalLength() * 0.001;
    }

    const auto& psActorsAndMappers = this->Importer->GetPointSpritesActorsAndMappers();
    for (auto& psActorAndMapper : psActorsAndMappers)
    {
      psActorAndMapper.second->SetScaleFactor(gaussianPointSize);
    }
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
  assert(this->Importer != nullptr);

  if (this->UseInverseOpacityFunction != use)
  {
    this->UseInverseOpacityFunction = use;

    const auto& volPropsAndMappers = this->Importer->GetVolumePropsAndMappers();
    for (auto& volPropAndMapper : volPropsAndMappers)
    {
      if (volPropAndMapper.first)
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
    case (CycleType::NONE):
      return;
      break;
    case (CycleType::FIELD):
      this->CycleFieldForColoring();
      break;
    case (CycleType::ARRAY_INDEX):
      this->CycleArrayIndexForColoring();
      break;
    case (CycleType::COMPONENT):
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
  assert(this->Importer != nullptr);

  // Never force change of anything if we are currently not coloring
  if (this->ArrayIndexForColoring == -1)
  {
    return CycleType::NONE;
  }

  // Never force change of CellData/PointData coloring on the user
  if (this->Importer->GetNumberOfIndexesForColoring(this->UseCellColoring) == 0)
  {
    return CycleType::NONE;
  }

  // Suggest to change the array index only if current index is not valid
  vtkF3DGenericImporter::ColoringInfo info;
  if (!this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    return CycleType::ARRAY_INDEX;
  }

  // Suggest to change the component if current component is invalid
  if (this->ComponentForColoring >= info.MaximumNumberOfComponents)
  {
    return CycleType::COMPONENT;
  }

  return CycleType::NONE;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::SetColoring(
  bool useCellData, const std::string& arrayName, int component)
{
  assert(this->Importer != nullptr);

  if (this->GetColoringUseCell() != useCellData || this->GetColoringArrayName() != arrayName ||
    this->GetColoringComponent() != component)
  {
    this->UseCellColoring = useCellData;

    int nIndexes = this->Importer->GetNumberOfIndexesForColoring(this->UseCellColoring);
    if (arrayName == F3D_RESERVED_STRING)
    {
      // Not coloring
      this->ArrayIndexForColoring = -1;
    }
    else if (arrayName != F3D_RESERVED_STRING && nIndexes == 0)
    {
      // Trying to color but no array available
      F3DLog::Print(F3DLog::Severity::Warning, "No array to color with");
      this->ArrayIndexForColoring = -1;
    }
    else if (arrayName.empty())
    {
      // Coloring with first array
      this->ArrayIndexForColoring = 0;
    }
    else
    {
      // Coloring with named array
      this->ArrayIndexForColoring = this->Importer->FindIndexForColoring(useCellData, arrayName);
      if (this->ArrayIndexForColoring == -1)
      {
        // Could not find named array
        F3DLog::Print(F3DLog::Severity::Warning, "Unknown scalar array: " + arrayName + "\n");
      }
    }

    // TODO rework this
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
  assert(this->Importer != nullptr);

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
  assert(this->Importer != nullptr);

  bool importerChanged = this->Importer->GetMTime() >= this->ColoringUpdateTime;

  // Early return if nothing changed
  if (!importerChanged && this->ColoringUpdateTime >= this->ColoringTimeStamp.GetMTime())
  {
    return;
  }

  if (importerChanged)
  {
    this->ColorTransferFunctionConfigured = false;
    this->PolyDataMapperConfigured = false;
    this->PointGaussianMapperConfigured = false;
    this->VolumeConfigured = false;
    this->ScalarBarActorConfigured = false;
  }

  this->ColoringUpdateTime = this->ColoringTimeStamp.GetMTime();

  // Recover coloring information
  vtkF3DGenericImporter::ColoringInfo info;
  bool hasColoring =
    this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info);

  bool volumeVisible = !this->UseRaytracing && this->UseVolume;
  if (!hasColoring && volumeVisible)
  {
    // When showing volume, always try to find an array to color with
    this->CycleScalars(vtkF3DRendererWithColoring::CycleType::ARRAY_INDEX);
    hasColoring =
      this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info);
  }

  if (hasColoring && !this->ColorTransferFunctionConfigured)
  {
    this->ConfigureRangeAndCTFForColoring(info);
    this->ColorTransferFunctionConfigured = true;
  }

  // Handle surface geometry
  bool geometriesVisible = this->UseRaytracing || (!this->UseVolume && !this->UsePointSprites);
  const auto& actorsAndMappers = this->Importer->GetGeometryActorsAndMappers();
  for (size_t i = 0; i < actorsAndMappers.size(); i++)
  {
    auto& actorAndMapper = actorsAndMappers[i];
    vtkDataArray* coloringArray = nullptr;
    if (hasColoring && info.Arrays.size() > i)
    {
      coloringArray = info.Arrays[i];
    }
    actorAndMapper.first->SetVisibility(geometriesVisible);
    if (geometriesVisible && coloringArray)
    {
      if (!this->PolyDataMapperConfigured)
      {
        vtkF3DRendererWithColoring::ConfigureMapperForColoring(actorAndMapper.second, coloringArray,
          this->ComponentForColoring, this->ColorTransferFunction, this->ColorRange,
          this->UseCellColoring);
        this->PolyDataMapperConfigured = true;
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
  const auto& psActorsAndMappers = this->Importer->GetPointSpritesActorsAndMappers();
  for (size_t i = 0; i < psActorsAndMappers.size(); i++)
  {
    auto& actorAndMapper = psActorsAndMappers[i];
    vtkDataArray* coloringArray = nullptr;
    if (hasColoring && info.Arrays.size() > i)
    {
      coloringArray = info.Arrays[i];
    }
    actorAndMapper.first->SetVisibility(pointSpritesVisible);
    if (pointSpritesVisible && coloringArray)
    {
      if (!this->PointGaussianMapperConfigured)
      {
        vtkF3DRendererWithColoring::ConfigureMapperForColoring(actorAndMapper.second, coloringArray,
          this->ComponentForColoring, this->ColorTransferFunction, this->ColorRange,
          this->UseCellColoring);
        this->PointGaussianMapperConfigured = true;
      }
      actorAndMapper.second->ScalarVisibilityOn();
    }
    else
    {
      actorAndMapper.second->ScalarVisibilityOff();
    }
  }

  // Handle Volume prop
  const auto& volPropsAndMappers = this->Importer->GetVolumePropsAndMappers();
  for (size_t i = 0; i < volPropsAndMappers.size(); i++)
  {
    auto& propAndMapper = volPropsAndMappers[i];
    vtkDataArray* coloringArray = nullptr;
    if (hasColoring && info.Arrays.size() > i)
    {
      coloringArray = info.Arrays[i];
    }
    if (!volumeVisible)
    {
      propAndMapper.first->VisibilityOff();
    }
    else if (volumeVisible && !coloringArray)
    {
      F3DLog::Print(
        F3DLog::Severity::Error, "Cannot use volume with this dataset or with the requested array");
      propAndMapper.first->VisibilityOff();
    }
    else
    {
      if (!this->VolumeConfigured)
      {
        vtkF3DRendererWithColoring::ConfigureVolumeForColoring(propAndMapper.second,
          propAndMapper.first, coloringArray, this->ComponentForColoring,
          this->ColorTransferFunction, this->ColorRange, this->UseCellColoring,
          this->UseInverseOpacityFunction);
        this->VolumeConfigured = true;
      }
      propAndMapper.first->VisibilityOn();
    }
  }

  // Handle scalar bar
  bool barVisible = this->ScalarBarVisible && hasColoring && this->ComponentForColoring >= -1;
  this->ScalarBarActor->SetVisibility(barVisible);
  if (barVisible && !this->ScalarBarActorConfigured)
  {
    vtkF3DRendererWithColoring::ConfigureScalarBarActorForColoring(
      this->ScalarBarActor, info.Name, this->ComponentForColoring, this->ColorTransferFunction);
    this->ScalarBarActorConfigured = true;
  }

  this->SetupRenderPasses();
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::GetColoringDescription()
{
  assert(this->Importer != nullptr);

  std::stringstream stream;
  vtkF3DGenericImporter::ColoringInfo info;
  if (this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    stream << "Coloring using " << (this->UseCellColoring ? "cell" : "point") << " array named "
           << info.Name << ", "
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
  vtkScalarBarActor* scalarBar, std::string arrayName, int component, vtkColorTransferFunction* ctf)
{
  arrayName += " (";
  arrayName += this->ComponentToString(component);
  arrayName += ")";

  scalarBar->SetLookupTable(ctf);
  scalarBar->SetTitle(arrayName.c_str());
  scalarBar->SetNumberOfLabels(4);
  scalarBar->SetOrientationToHorizontal();
  scalarBar->SetWidth(0.8);
  scalarBar->SetHeight(0.07);
  scalarBar->SetPosition(0.1, 0.01);
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::ConfigureRangeAndCTFForColoring(
  const vtkF3DGenericImporter::ColoringInfo& info)
{
  if (this->ComponentForColoring == -2) // TODO use options
  {
    return;
  }

  if (this->ComponentForColoring >= info.MaximumNumberOfComponents)
  {
    F3DLog::Print(F3DLog::Severity::Warning,
      std::string("Invalid component index: ") + std::to_string(this->ComponentForColoring) + "\n");
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
void vtkF3DRendererWithColoring::FillCheatSheetHotkeys(std::stringstream& cheatSheetText)
{
  assert(this->Importer != nullptr);

  vtkF3DGenericImporter::ColoringInfo info;
  bool hasColoring =
    this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info);

  cheatSheetText << " C: Cell scalars coloring [" << (this->UseCellColoring ? "ON" : "OFF")
                 << "]\n";
  cheatSheetText << " S: Scalars coloring ["
                 << (hasColoring ? vtkF3DRendererWithColoring::ShortName(info.Name, 19) : "OFF")
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
void vtkF3DRendererWithColoring::CycleFieldForColoring()
{
  // A generic approach will be better when adding categorical field data coloring
  this->UseCellColoring = !this->UseCellColoring;
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleArrayIndexForColoring()
{
  assert(this->Importer != nullptr);

  int nIndex = this->Importer->GetNumberOfIndexesForColoring(this->UseCellColoring);
  if (nIndex <= 0)
  {
    return;
  }

  if (this->UseVolume)
  {
    this->ArrayIndexForColoring = (this->ArrayIndexForColoring + 1) % nIndex;
  }
  else
  {
    // Cycle through arrays looping back to -1
    // -1 0 1 2 -1 0 1 2 ...
    this->ArrayIndexForColoring = (this->ArrayIndexForColoring + 2) % (nIndex + 1) - 1;
  }
}

//----------------------------------------------------------------------------
void vtkF3DRendererWithColoring::CycleComponentForColoring()
{
  assert(this->Importer != nullptr);

  vtkF3DGenericImporter::ColoringInfo info;
  if (!this->Importer->GetInfoForColoring(this->UseCellColoring, this->ArrayIndexForColoring, info))
  {
    return;
  }

  // -2 -1 0 1 2 3 4
  this->ComponentForColoring =
    (this->ComponentForColoring + 3) % (info.MaximumNumberOfComponents + 2) -
    2; // TODO separate direct scalars and magnitude from component number
}

//----------------------------------------------------------------------------
std::string vtkF3DRendererWithColoring::GenerateMetaDataDescription()
{
  assert(this->Importer != nullptr);

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
  assert(this->Importer != nullptr);

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
    if (!this->Importer->GetInfoForColoring(
          this->UseCellColoring, this->ArrayIndexForColoring, info))
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
