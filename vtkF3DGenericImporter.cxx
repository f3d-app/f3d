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

#include "vtkActor.h"
#include "vtkAppendPolyData.h"
#include "vtkDataObjectTreeRange.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkObjectFactory.h"
#include "vtkLightKit.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"

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

    for (vtkDataObject* current : vtk::Range(mb, vtk::DataObjectTreeOptions::SkipEmptyNodes |
                                                 vtk::DataObjectTreeOptions::TraverseSubTree |
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
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportLights(vtkRenderer* ren)
{
  ren->AutomaticLightCreationOff();

  vtkNew<vtkLightKit> lightKit;
  lightKit->AddLightsToRenderer(ren);
}

//----------------------------------------------------------------------------
void vtkF3DGenericImporter::ImportProperties(vtkRenderer* ren)
{
  ren->SetBackground(.2, .2, .2);
  ren->UseFXAAOn();
}

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
