#include "F3DViewer.h"

#include "F3DOptions.h"

#include "vtkF3DGenericImporter.h"
#include "vtkF3DOpenGLGridMapper.h"

#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkBoundingBox.h>
#include <vtkPointSource.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>


//----------------------------------------------------------------------------
F3DViewer::F3DViewer(F3DOptions *options, vtkF3DGenericImporter *importer)
{
  this->Options = options;
  this->Importer = importer;

  this->RenderWindow = importer->GetRenderWindow();

  this->Renderer = this->RenderWindow->GetRenderers()->GetFirstRenderer();

  this->RenderWindowInteractor->SetRenderWindow(this->RenderWindow);
  this->RenderWindowInteractor->SetInteractorStyle(this->InteractorStyle);

  this->InteractorStyle->SetViewer(this);

  this->RenderWindow->SetSize(this->Options->WindowSize[0], this->Options->WindowSize[1]);
  this->RenderWindow->SetWindowName(f3d::AppTitle.c_str());
}

//----------------------------------------------------------------------------
void F3DViewer::ShowAxis(bool show)
{
  vtkNew<vtkAxesActor> axes;
  this->AxisWidget->SetOrientationMarker(axes);
  this->AxisWidget->SetInteractor(this->RenderWindowInteractor);
  this->AxisWidget->SetViewport(0.85, 0.0, 1.0, 0.15);
  this->AxisWidget->InteractiveOff();
  this->AxisWidget->SetEnabled(show);
}

//----------------------------------------------------------------------------
bool F3DViewer::IsAxisVisible()
{
  return this->AxisWidget->GetEnabled();
}

//----------------------------------------------------------------------------
void F3DViewer::ShowGrid(bool show)
{
  this->Renderer->SetClippingRangeExpansion(0.99);
  this->Renderer->ResetCameraClippingRange();

  double bounds[6];
  this->Renderer->ComputeVisiblePropBounds(bounds);

  vtkBoundingBox bbox(bounds);

  vtkNew<vtkPointSource> gridPointSource;
  gridPointSource->SetNumberOfPoints(1);
  gridPointSource->SetRadius(0);
  gridPointSource->SetCenter(0, bounds[2], 0);

  vtkNew<vtkF3DOpenGLGridMapper> gridMapper;
  gridMapper->SetInputConnection(gridPointSource->GetOutputPort());
  gridMapper->SetFadeDistance(bbox.GetDiagonalLength());

  this->GridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
  this->GridActor->SetMapper(gridMapper);

  this->Renderer->RemoveActor(this->GridActor);
  this->Renderer->AddActor(this->GridActor);

  this->GridActor->SetVisibility(show);
}

//----------------------------------------------------------------------------
bool F3DViewer::IsGridVisible()
{
  return this->GridActor->GetVisibility();
}

//----------------------------------------------------------------------------
void F3DViewer::SetupWithOptions()
{
  this->ShowGrid(this->Options->Grid);
  this->ShowAxis(this->Options->Axis);

  this->Render();
}

//----------------------------------------------------------------------------
void F3DViewer::Start()
{
  this->SetupWithOptions();

  this->RenderWindowInteractor->Start();
}

//----------------------------------------------------------------------------
void F3DViewer::Render()
{
  this->RenderWindow->Render();
}
