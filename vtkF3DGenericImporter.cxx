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
#include "vtkCellData.h"
#include "vtkDataObjectTreeRange.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkLightKit.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkScalarBarActor.h"
#include "vtkScalarsToColors.h"

vtkStandardNewMacro(vtkF3DGenericImporter);

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportActors(vtkRenderer* ren)
{
  this->Reader->Update();

  vtkNew<vtkPolyDataMapper> mapper;

  vtkMultiBlockDataSet* mb = vtkMultiBlockDataSet::SafeDownCast(this->Reader->GetOutput());
  if (mb)
  {
    vtkNew<vtkAppendPolyData> append;

    for (vtkDataObject* current :
      vtk::Range(mb,
        vtk::DataObjectTreeOptions::SkipEmptyNodes | vtk::DataObjectTreeOptions::TraverseSubTree |
          vtk::DataObjectTreeOptions::VisitOnlyLeaves))
    {
      vtkNew<vtkDataSetSurfaceFilter> geom;
      geom->SetInputData(current);
      geom->Update();
      append->AddInputData(vtkPolyData::SafeDownCast(geom->GetOutput()));
    }

    mapper->SetInputConnection(append->GetOutputPort());
  }
  else
  {
    vtkNew<vtkDataSetSurfaceFilter> geom;
    geom->SetInputConnection(this->Reader->GetOutputPort());
    mapper->SetInputConnection(geom->GetOutputPort());
  }

  vtkNew<vtkActor> actor;
  actor->GetProperty()->SetInterpolationToPBR();
  actor->GetProperty()->SetRoughness(0.3);
  actor->SetMapper(mapper);

  ren->AddActor(actor);

  mapper->Update();
  vtkPolyData* surface = vtkPolyData::SafeDownCast(mapper->GetInput());

  if (surface && this->Options)
  {
    vtkPointData* pointData = surface->GetPointData();
    vtkIdType nbPointData = pointData->GetNumberOfArrays();
    vtkCellData* cellData = surface->GetCellData();
    vtkIdType nbCellData = cellData->GetNumberOfArrays();

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
        cout << "Using default scalar array: " << usedArray << endl;
      }
      else
      {
        usedArray = "";
        cout << "No default scalar array, please specify an array name." << endl;
      }
    }

    if (!usedArray.empty())
    {
      mapper->ScalarVisibilityOn();
      mapper->InterpolateScalarsBeforeMappingOn();
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
        vtkDataArray* array = this->Options->Cells
          ? cellData->GetArray(usedArray.c_str())
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
        title += " (";
        title += std::to_string(this->Options->Component);
        title += ")";
      }
      else
      {
        title += " (Magnitude)";
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
  }
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
