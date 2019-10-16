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

#include "F3DOptions.h"

#include "vtkActor.h"
#include "vtkAppendPolyData.h"
#include "vtkBoundingBox.h"
#include "vtkCellData.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkEventForwarderCommand.h"
#include "vtkLightKit.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointGaussianMapper.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkScalarBarActor.h"
#include "vtkScalarsToColors.h"

vtkStandardNewMacro(vtkF3DGenericImporter);

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportActors(vtkRenderer* ren)
{
  if (!this->Reader->IsReaderValid())
  {
    cerr << "File \"" << this->Reader->GetFileName() << "\" cannot be read." << endl;
    return;
  }

  // forward progress event
  vtkNew<vtkEventForwarderCommand> forwarder;
  forwarder->SetTarget(this);
  this->Reader->AddObserver(vtkCommand::ProgressEvent, forwarder);

  this->Reader->Update();

  vtkSmartPointer<vtkDataObject> dataObject = this->Reader->GetOutput();

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

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

  vtkPointData* pointData = surface->GetPointData();
  vtkIdType nbPointData = pointData->GetNumberOfArrays();
  vtkCellData* cellData = surface->GetCellData();
  vtkIdType nbCellData = cellData->GetNumberOfArrays();

  if (surface->GetNumberOfVerts() == surface->GetNumberOfCells())
  {
    double bounds[6];
    surface->GetBounds(bounds);

    vtkBoundingBox bbox(bounds);
    vtkNew<vtkPointGaussianMapper> gaussianMapper;
    gaussianMapper->SetScaleFactor(bbox.GetDiagonalLength() * 0.02);
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

  mapper->SetInputData(surface);
  mapper->Update();

  if (this->Options->Verbose)
  {
    cout << "Number of points: " << surface->GetNumberOfPoints()
         << "\nNumber of polygons: " << surface->GetNumberOfPolys()
         << "\nNumber of lines: " << surface->GetNumberOfLines()
         << "\nNumber of vertices: " << surface->GetNumberOfVerts() << "\n";

    cout << nbPointData << " point data array(s)\n";
    for (vtkIdType i = 0; i < nbPointData; i++)
    {
      vtkDataArray* array = pointData->GetArray(i);
      cout << " #" << i << " '" << array->GetName() << "': " << array->GetNumberOfComponents()
           << " comp.\n";
    }

    cout << nbCellData << " cell data array(s)\n";
    for (vtkIdType i = 0; i < nbCellData; i++)
    {
      vtkDataArray* array = cellData->GetArray(i);
      cout << " #" << i << " '" << array->GetName() << "': " << array->GetNumberOfComponents()
           << " comp.\n";
    }

    cout.flush();
  }

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
        cout << "Using default scalar array: " << usedArray << endl;
      }
    }
    else
    {
      usedArray = "";
      if (this->Options->Verbose)
      {
        cout << "No default scalar array, please specify an array name." << endl;
      }
    }
  }

  if (!usedArray.empty())
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
      vtkDataArray* array = this->Options->Cells ? cellData->GetArray(usedArray.c_str())
                                                 : pointData->GetArray(usedArray.c_str());

      if (array)
      {
        double range[2];
        array->GetRange(range, this->Options->Component);
        mapper->SetScalarRange(range);
      }
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
    scalarBar->SetVisibility(!this->Options->HideBar);

    ren->AddActor2D(scalarBar);
  }
  else
  {
    mapper->ScalarVisibilityOff();
  }

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

#if VTK_VERSION_MAJOR == 8 && VTK_VERSION_MINOR > 2
  actor->GetProperty()->SetInterpolationToPBR();
  actor->GetProperty()->SetRoughness(0.3);
#endif

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
