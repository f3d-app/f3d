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

  if (this->Options->Verbose)
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

  vtkSmartPointer<vtkPolyDataMapper> mapper;

  vtkPointData* pointData = surface->GetPointData();
  vtkCellData* cellData = surface->GetCellData();
  if (!this->Options->Raytracing && this->Options->PointSprites)
  {
    double bounds[6];
    surface->GetBounds(bounds);

    vtkBoundingBox bbox(bounds);
    vtkNew<vtkPointGaussianMapper> gaussianMapper;

    double pointSize = this->Options->PointSize * bbox.GetDiagonalLength() * 0.001;

    gaussianMapper->SetScaleFactor(pointSize);
    gaussianMapper->EmissiveOff();
    gaussianMapper->SetSplatShaderCode(
      "//VTK::Color::Impl\n"
      "float dist = dot(offsetVCVSOutput.xy, offsetVCVSOutput.xy);\n"
      "if (dist > 1.0) {\n"
      "  discard;\n"
      "} else {\n"
      "  float scale = (1.0 - dist);\n"
      "  ambientColor *= scale;\n"
      "  diffuseColor *= scale;\n"
      "}\n");
    mapper = gaussianMapper;
  }
  else
  {
    mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->InterpolateScalarsBeforeMappingOn();
    mapper->SetColorModeToMapScalars();
  }

  mapper->SetInputData(surface);
  mapper->Update();

  std::string usedArray = this->Options->Scalars;

  if (usedArray == "f3d_reserved")
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

    if (array)
    {
      usedArray = array->GetName();
      if (this->Options->Verbose)
      {
        F3DLog::Print(F3DLog::Severity::Info, "Using default scalar array: ", usedArray);
      }
    }
    else
    {
      usedArray = "";
      if (this->Options->Verbose)
      {
        F3DLog::Print(F3DLog::Severity::Info, "No default scalar array, please specify an array name.");
      }
    }
  }

  mapper->ScalarVisibilityOff();
  if (!usedArray.empty())
  {
    vtkDataArray* array = this->Options->Cells ? cellData->GetArray(usedArray.c_str())
                                               : pointData->GetArray(usedArray.c_str());

    if (array)
    {
      if (this->Options->Component < array->GetNumberOfComponents())
      {
        mapper->ScalarVisibilityOn();
        mapper->SelectColorArray(usedArray.c_str());
        mapper->SetScalarMode(this->Options->Cells ? VTK_SCALAR_MODE_USE_CELL_FIELD_DATA
                                                   : VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);

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

        if (this->Options->Range.size() == 2)
        {
          mapper->SetScalarRange(this->Options->Range[0], this->Options->Range[1]);
        }
        else
        {
          double range[2];
          array->GetRange(range, this->Options->Component);
          mapper->SetScalarRange(range);
        }

        std::string title = usedArray;
        if (this->Options->Component >= 0)
        {
          title += " (Component #";
          title += std::to_string(this->Options->Component);
          title += ")";
        }

        vtkNew<vtkScalarBarActor> scalarBar;
        scalarBar->SetLookupTable(lut);
        scalarBar->SetTitle(title.c_str());
        scalarBar->SetNumberOfLabels(4);
        scalarBar->SetOrientationToHorizontal();
        scalarBar->SetWidth(0.8);
        scalarBar->SetHeight(0.07);
        scalarBar->SetPosition(0.1, 0.01);
        scalarBar->SetVisibility(this->Options->Bar);

        ren->AddActor2D(scalarBar);
      }
      else
      {
        F3DLog::Print(F3DLog::Severity::Warning, "Invalid component index: ", this->Options->Component);
      }
    }
    else
    {
      F3DLog::Print(F3DLog::Severity::Warning, "Unknow scalar array: ", usedArray);
    }
  }

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  actor->GetProperty()->SetInterpolationToPBR();

  double col[3];
  std::copy(this->Options->SolidColor.begin(), this->Options->SolidColor.end(), col);

  actor->GetProperty()->SetColor(col);
  actor->GetProperty()->SetOpacity(this->Options->Opacity);
  actor->GetProperty()->SetRoughness(this->Options->Roughness);
  actor->GetProperty()->SetMetallic(this->Options->Metallic);
  actor->GetProperty()->SetPointSize(this->Options->PointSize);

  ren->AddActor(actor);
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportLights(vtkRenderer* ren)
{
  ren->AutomaticLightCreationOff();

  vtkNew<vtkLightKit> lightKit;
  lightKit->AddLightsToRenderer(ren);
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportProperties(vtkRenderer* ren) {}

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
  for (int i = 0; i < mb->GetNumberOfBlocks(); i++)
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
