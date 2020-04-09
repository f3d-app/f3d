/*=========================================================================
  Program:   Visualization Toolkit
  Module:    vtkF3DGenericImporter.cxx
  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.
=========================================================================*/

#include "vtkF3DGenericImporter.h"

#include "F3DLog.h"
#include "F3DOptions.h"

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkBoundingBox.h>
#include <vtkCellData.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkEventForwarderCommand.h>
#include <vtkInformation.h>
#include <vtkLightKit.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarsToColors.h>

vtkStandardNewMacro(vtkF3DGenericImporter);

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportActors(vtkRenderer* ren)
{
  if (!this->Reader->IsReaderValid())
  {
    F3DLog::Print(
      F3DLog::Severity::Info, "File '", this->Reader->GetFileName(), "' cannot be read.");
    return;
  }

  // forward progress event
  vtkNew<vtkEventForwarderCommand> forwarder;
  forwarder->SetTarget(this);
  this->Reader->AddObserver(vtkCommand::ProgressEvent, forwarder);

  this->Reader->Update();

  vtkSmartPointer<vtkDataObject> dataObject = this->Reader->GetOutput();

  if (this->Options->Verbose || this->Options->NoRender)
  {
    this->OutputDescription = vtkF3DGenericImporter::GetDataObjectDescription(dataObject);
  }

  vtkMultiBlockDataSet* mb = vtkMultiBlockDataSet::SafeDownCast(dataObject);
  if (mb)
  {
    vtkNew<vtkAppendPolyData> append;

    auto iter = vtkSmartPointer<vtkDataObjectTreeIterator>::Take(mb->NewTreeIterator());
    iter->VisitOnlyLeavesOn();
    iter->SkipEmptyNodesOn();
    iter->TraverseSubTreeOn();

    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
      vtkNew<vtkDataSetSurfaceFilter> geom;
      geom->SetInputData(iter->GetCurrentDataObject());
      geom->Update();
      append->AddInputData(vtkPolyData::SafeDownCast(geom->GetOutput()));
    }

    append->Update();
    dataObject = append->GetOutput();
  }
  else
  {
    vtkNew<vtkDataSetSurfaceFilter> geom;
    geom->SetInputConnection(this->Reader->GetOutputPort());
    geom->Update();
    dataObject = geom->GetOutput();
  }

  vtkPolyData* surface = vtkPolyData::SafeDownCast(dataObject);

  if (!surface || !this->Options)
  {
    return;
  }

  // Configure polydata mapper
  this->PolyDataMapper->InterpolateScalarsBeforeMappingOn();
  this->PolyDataMapper->SetColorModeToMapScalars();
  this->PolyDataMapper->SetInputData(surface);

  // Configure Point Gaussian mapper
  double bounds[6];
  surface->GetBounds(bounds);
  vtkBoundingBox bbox(bounds);
  double pointSize = this->Options->PointSize * bbox.GetDiagonalLength() * 0.001;
  this->PointGaussianMapper->SetScaleFactor(pointSize);
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
  this->PointGaussianMapper->SetInputData(surface);

  // Select correct mapper
  vtkSmartPointer<vtkPolyDataMapper> mapper;
  if (!this->Options->Raytracing && this->Options->PointSprites)
  {
    mapper = this->PointGaussianMapper;
  }
  else
  {
    mapper = this->PolyDataMapper;
  }

  vtkPointData* pointData = surface->GetPointData();
  vtkCellData* cellData = surface->GetCellData();
  std::string usedArray = this->Options->Scalars;

  // Recover an array for coloring if we ever need it
  if (usedArray.empty() || usedArray == f3d::F3DReservedString)
  {
    vtkDataArray* array = nullptr;
    if (this->Options->Cells)
    {
      array = cellData->GetScalars();
    }
    else
    {
      array = pointData->GetScalars();
    }

    bool print = (this->Options->Verbose || this->Options->NoRender);
    if (array)
    {
      usedArray = array->GetName();
      if (print)
      {
        F3DLog::Print(F3DLog::Severity::Info, "Using default scalar array: ", usedArray);
      }
    }
    else
    {
      if (this->Options->Cells && cellData->GetNumberOfArrays() > 0)
      {
        array = cellData->GetArray(0);
      }
      else if (!this->Options->Cells && pointData->GetNumberOfArrays() > 0)
      {
        array = pointData->GetArray(0);
      }

      if (array)
      {
        usedArray = array->GetName();
        if (print)
        {
          F3DLog::Print(F3DLog::Severity::Info, "Using first found array: ", usedArray);
        }
      }
      else
      {
        usedArray = "";
        if (print)
        {
          F3DLog::Print(F3DLog::Severity::Info, "No array found for scalar coloring");
        }
      }
    }
  }

  // Configure the mappers for coloring if it is ever needed
  vtkDataArray* array = this->Options->Cells ? cellData->GetArray(usedArray.c_str())
    : pointData->GetArray(usedArray.c_str());
  if (array)
  {
    if (this->Options->Component < array->GetNumberOfComponents())
    {
      this->ConfigureMapperForColoring(this->PointGaussianMapper, array);
      this->ConfigureMapperForColoring(this->PolyDataMapper, array);

      vtkScalarsToColors* lut = mapper->GetLookupTable();

      std::string title = usedArray;
      if (this->Options->Component >= 0)
      {
        title += " (Component #";
        title += std::to_string(this->Options->Component);
        title += ")";
      }

      this->ScalarBarActor->SetLookupTable(lut);
      this->ScalarBarActor->SetTitle(title.c_str());
      this->ScalarBarActor->SetNumberOfLabels(4);
      this->ScalarBarActor->SetOrientationToHorizontal();
      this->ScalarBarActor->SetWidth(0.8);
      this->ScalarBarActor->SetHeight(0.07);
      this->ScalarBarActor->SetPosition(0.1, 0.01);
      this->ScalarBarActor->SetVisibility(this->Options->Bar);
      ren->AddActor2D(this->ScalarBarActor);
    }
    else
    {
      F3DLog::Print(F3DLog::Severity::Warning, "Invalid component index: ", this->Options->Component);
    }
  }
  else if (!usedArray.empty() && !(usedArray == f3d::F3DReservedString))
  {
    F3DLog::Print(F3DLog::Severity::Warning, "Unknow scalar array: ", usedArray);
  }

  this->GeometryActor->SetMapper(mapper);
  this->GeometryActor->GetProperty()->SetInterpolationToPBR();

  double col[3];
  std::copy(this->Options->SolidColor.begin(), this->Options->SolidColor.end(), col);

  this->GeometryActor->GetProperty()->SetColor(col);
  this->GeometryActor->GetProperty()->SetOpacity(this->Options->Opacity);
  this->GeometryActor->GetProperty()->SetRoughness(this->Options->Roughness);
  this->GeometryActor->GetProperty()->SetMetallic(this->Options->Metallic);
  this->GeometryActor->GetProperty()->SetPointSize(this->Options->PointSize);

  ren->AddActor(this->GeometryActor);
}

//----------------------------------------------------------------------------
vtkScalarsToColors* vtkF3DGenericImporter::ConfigureMapperForColoring(vtkMapper* mapper, vtkDataArray* array)
{
  mapper->SelectColorArray(array->GetName());
  mapper->SetScalarMode(this->Options->Cells ? VTK_SCALAR_MODE_USE_CELL_FIELD_DATA
                        : VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
  mapper->SetScalarVisibility(this->Options->Scalars != f3d::F3DReservedString);

  double range[2];
  if (this->Options->Range.size() == 2)
  {
    range[0] = this->Options->Range[0];
    range[1] = this->Options->Range[1];
  }
  else
  {
    if (this->Options->Range.size() > 0)
    {
      F3DLog::Print(F3DLog::Severity::Warning, "The range specified does not have exactly 2 values");
    }
    array->GetRange(range, this->Options->Component);
  }
  mapper->SetScalarRange(range);

  if (this->Options->LookupPoints.size() > 0)
  {
    if (this->Options->LookupPoints.size() % 4 == 0)
    {
      vtkNew<vtkColorTransferFunction> ctf;
      for (size_t i = 0; i < this->Options->LookupPoints.size(); i += 4)
      {
        double val = this->Options->LookupPoints[i];
        double r = this->Options->LookupPoints[i + 1];
        double g = this->Options->LookupPoints[i + 2];
        double b = this->Options->LookupPoints[i + 3];
        ctf->AddRGBPoint(range[0] + val * (range[1] - range[0]), r, g, b);
      }
      mapper->SetLookupTable(ctf);
    }
    else
    {
      F3DLog::Print(F3DLog::Severity::Warning, "Specified color map list count is not a multiple of 4");
    }
  }

  vtkScalarsToColors* lut = mapper->GetLookupTable();

  if (this->Options->Component >= 0)
  {
    lut->SetVectorModeToComponent();
    lut->SetVectorComponent(this->Options->Component);
  }
  else
  {
    lut->SetVectorModeToMagnitude();
  }

  return lut;
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportLights(vtkRenderer* ren)
{
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
  this->Reader->SetFileName(std::string(arg));
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::SetOptions(const F3DOptions& options)
{
  this->Options = &options;
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
std::string vtkF3DGenericImporter::GetMultiBlockDescription(vtkMultiBlockDataSet* mb, vtkIndent indent)
{
  std::stringstream ss;
  for (vtkIdType i = 0; i < mb->GetNumberOfBlocks(); i++)
  {
    const char* blockName = mb->GetMetaData(i)->Get(vtkCompositeDataSet::NAME());
    ss << indent << "Block: " << (blockName ? std::string(blockName) : std::to_string(i))<< "\n";
    vtkDataObject* object = mb->GetBlock(i);
    vtkMultiBlockDataSet* mbChild = vtkMultiBlockDataSet::SafeDownCast(object);
    vtkDataSet* ds = vtkDataSet::SafeDownCast(object);
    if (mbChild)
    {
      ss << vtkF3DGenericImporter::GetMultiBlockDescription(mbChild, indent.GetNextIndent());
    }
    else if(ds)
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
