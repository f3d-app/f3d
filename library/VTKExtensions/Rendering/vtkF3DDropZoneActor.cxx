#include "vtkF3DDropZoneActor.h"

#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkViewport.h>

vtkStandardNewMacro(vtkF3DDropZoneActor);

//------------------------------------------------------------------------------
vtkF3DDropZoneActor::vtkF3DDropZoneActor()
{
  this->TextActor->SetMapper(this->TextMapper);
  vtkTextProperty* tProp = this->TextMapper->GetTextProperty();
  tProp->SetFontSize(25);
  tProp->SetJustificationToCentered();
  tProp->SetVerticalJustificationToCentered();

  this->BorderMapper->SetInputData(this->BorderPolyData);
  this->BorderActor->SetMapper(this->BorderMapper);
}

//------------------------------------------------------------------------------
vtkF3DDropZoneActor::~vtkF3DDropZoneActor() = default;

//------------------------------------------------------------------------------
void vtkF3DDropZoneActor::ReleaseGraphicsResources(vtkWindow* win)
{
  this->TextActor->ReleaseGraphicsResources(win);
  this->BorderActor->ReleaseGraphicsResources(win);
}

//------------------------------------------------------------------------------
int vtkF3DDropZoneActor::RenderOverlay(vtkViewport* viewport)
{
  const int* vSize = viewport->GetSize();

  // Simply position text actor in the middle
  this->TextMapper->SetInput(this->DropText.c_str());
  this->TextActor->SetPosition(vSize[0] / 2, vSize[1] / 2);
  this->TextActor->RenderOverlay(viewport);

  if (this->BuildBorderGeometry(viewport))
  {
    this->BorderActor->RenderOverlay(viewport);
  }
  return 1;
}

//------------------------------------------------------------------------------
bool vtkF3DDropZoneActor::BuildBorderGeometry(vtkViewport* viewport)
{
  constexpr int tickWidth = 3;
  constexpr int tickLength = 10;

  const int* vSize = viewport->GetSize();
  if (vSize[0] == this->ComputedBorderViewportSize[0] &&
    vSize[1] == this->ComputedBorderViewportSize[1])
  {
    return true;
  }

  // padding is 10% of shortest side
  const int padding = std::min(vSize[0], vSize[1]) * 0.1;
  const int borderW = vSize[0] - 2 * padding;
  const int borderH = vSize[1] - 2 * padding;

  // number of ticks assuming spacing == tickLength
  const int nTicksW = borderW / (tickLength * 2);
  const int nTicksH = borderH / (tickLength * 2);

  // recompute uniform spacing
  const double spacingW = static_cast<double>(borderW - nTicksW * tickLength) / (nTicksW - 1);
  const double spacingH = static_cast<double>(borderH - nTicksH * tickLength) / (nTicksH - 1);

  vtkNew<vtkPoints> borderPoints;
  vtkNew<vtkCellArray> borderCells;
  borderPoints->SetNumberOfPoints(4 * (2 * nTicksW + 2 * nTicksH));
  vtkIdType index = 0;

  // Draw top/bottom
  for (int i = 0; i < nTicksW; ++i)
  {
    const int x0 = padding + tickLength * i + spacingW * i;
    const int x1 = x0 + tickLength;
    const int y[2] = { padding, vSize[1] - padding };
    const int h[2] = { +tickWidth, -tickWidth };
    for (int j = 0; j < 2; ++j)
    {
      vtkIdType ids[4] = { index++, index++, index++, index++ };
      borderPoints->SetPoint(ids[0], x0, y[j] + h[j], 0.);
      borderPoints->SetPoint(ids[1], x1, y[j] + h[j], 0.);
      borderPoints->SetPoint(ids[2], x1, y[j], 0.);
      borderPoints->SetPoint(ids[3], x0, y[j], 0.);
      borderCells->InsertNextCell(4, ids);
    }
  }

  // Draw left/right
  for (int i = 0; i < nTicksH; ++i)
  {
    const int y0 = padding + tickLength * i + spacingH * i;
    const int y1 = y0 + tickLength;
    const int x[2] = { padding, vSize[0] - padding };
    const int w[2] = { +tickWidth, -tickWidth };
    for (int j = 0; j < 2; ++j)
    {
      vtkIdType ids[4] = { index++, index++, index++, index++ };
      borderPoints->SetPoint(ids[0], x[j], y0, 0.);
      borderPoints->SetPoint(ids[1], x[j] + w[j], y0, 0.);
      borderPoints->SetPoint(ids[2], x[j] + w[j], y1, 0.);
      borderPoints->SetPoint(ids[3], x[j], y1, 0.);
      borderCells->InsertNextCell(4, ids);
    }
  }

  this->BorderPolyData->SetPoints(borderPoints);
  this->BorderPolyData->SetPolys(borderCells);

  this->ComputedBorderViewportSize[0] = vSize[0];
  this->ComputedBorderViewportSize[1] = vSize[1];
  return true;
}

//------------------------------------------------------------------------------
vtkTextProperty* vtkF3DDropZoneActor::GetTextProperty()
{
  return this->TextMapper->GetTextProperty();
}
