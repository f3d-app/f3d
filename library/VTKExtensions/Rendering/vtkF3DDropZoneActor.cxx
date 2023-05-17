#include "vtkF3DDropZoneActor.h"

#include <vtkObjectFactory.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkViewport.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>

constexpr int tickWidth = 3;
constexpr int tickLength = 10;

vtkStandardNewMacro(vtkF3DDropZoneActor);

//------------------------------------------------------------------------------
vtkF3DDropZoneActor::vtkF3DDropZoneActor()
{
  this->TextActor->SetMapper(this->TextMapper);
  this->TextMapper->SetInput("Drop a file to open it\nPress H to show cheatsheet");
  vtkTextProperty* tProp = this->TextMapper->GetTextProperty();
  tProp->SetFontSize(25);
  tProp->SetJustificationToCentered();
  tProp->SetVerticalJustificationToCentered();

  this->BorderMapper->SetInputData(this->BorderPolyData);
  this->BorderActor->SetMapper(this->BorderMapper);

}

//------------------------------------------------------------------------------
vtkF3DDropZoneActor::~vtkF3DDropZoneActor()
{
}

void vtkF3DDropZoneActor::ReleaseGraphicsResources(vtkWindow* win)
{
  this->TextActor->ReleaseGraphicsResources(win);
}

//------------------------------------------------------------------------------
int vtkF3DDropZoneActor::RenderOverlay(vtkViewport* viewport)
{
  const int* vSize = viewport->GetSize();

  // Simply position text actor in the middle
  this->TextActor->SetPosition(vSize[0] / 2, vSize[1] / 2);
  this->TextActor->RenderOverlay(viewport);

  this->BuildBorderGeometry(viewport);
  this->BorderActor->RenderOverlay(viewport);
  return 1;
}

//------------------------------------------------------------------------------
void vtkF3DDropZoneActor::BuildBorderGeometry(vtkViewport* viewport)
{
  const int* vSize = viewport->GetSize();
  if (vSize[0] == this->ComputedBorderViewportSize[0] &&
      vSize[1] == this->ComputedBorderViewportSize[1])
  {
    return;
  }

  // Create a border at 80% of the viewport
  vtkNew<vtkPoints> borderPoints;
  vtkNew<vtkCellArray> borderCells;
  vtkIdType pointCnt = 0;

  int borderCorner [2];
  borderCorner[0] = vSize[0] * 0.1;
  borderCorner[1] = vSize[1] * 0.1;
  int borderSize[2];
  borderSize[0] = vSize[0] * 0.8;
  borderSize[1] = vSize[1] * 0.8;

  // Determine the number of "ticks" by side, excluding corners, one "tick" is tickLength pixels long, tickWidth pixels wide
  int nTicksSides [2];
  nTicksSides[0] = borderSize[0] / (2 * tickLength) - 1;
  nTicksSides[1] = borderSize[1] / (2 * tickLength) - 1;

  // Compute the remaining offsets to equalize it between corners
  int emptyOffsets [2];
  emptyOffsets[0] = (borderSize[0] - (nTicksSides[0] + 1) * (2 * tickLength)) / 4;
  emptyOffsets[1] = (borderSize[1] - (nTicksSides[1] + 1) * (2 * tickLength)) / 4;

  // left/right, up/down and four corners
  borderPoints->SetNumberOfPoints(4 * (nTicksSides[0] * 2 + nTicksSides[1] * 2) + 6 * 4);

  for (int iSide = 0; iSide < 4; iSide++)
  {
    int xTag = iSide % 2; // 0 1 0 1
    int yTag = ((iSide + 1) / 2) % 2; // 0 1 1 0

    int xMult = 1 - xTag - yTag; // 1 -1 0 0
    int yMult = xTag - yTag; // 0 0 -1 1

    // Compute the corner point
    int basePoint[2];
    basePoint[0] = borderCorner[0] + borderSize[0] * xTag;
    basePoint[1] = borderCorner[1] + borderSize[1] * yTag;

    // Create corner geometries. A corner is a L shaped geometry, tickWidth pixels wides, tickLength pixels on first side, tickLength pixels on second side
    vtkIdType ids[6];
    ids[0] = pointCnt;
    borderPoints->SetPoint(pointCnt++, basePoint[0], basePoint[1], 0.);
    ids[1] = pointCnt;
    borderPoints->SetPoint(pointCnt++, basePoint[0] + tickLength * xMult, basePoint[1] + tickLength * yMult, 0.);
    ids[2] = pointCnt;
    borderPoints->SetPoint(pointCnt++, basePoint[0] + tickLength * xMult - tickWidth * yMult, basePoint[1] + tickWidth * xMult + tickLength * yMult, 0.);
    ids[3] = pointCnt;
    borderPoints->SetPoint(pointCnt++, basePoint[0] + tickWidth * xMult - tickWidth * yMult, basePoint[1] + tickWidth * xMult + tickWidth * yMult, 0.);
    ids[4] = pointCnt;
    borderPoints->SetPoint(pointCnt++, basePoint[0] + tickWidth * xMult - tickLength * yMult, basePoint[1] + tickLength * xMult + tickWidth * yMult, 0.);
    ids[5] = pointCnt;
    borderPoints->SetPoint(pointCnt++, basePoint[0] - tickLength * yMult, basePoint[1] + tickLength * xMult, 0.);
    borderCells->InsertNextCell(6, ids);

    // Move base point to first tick of the side
    basePoint[0] += (tickLength * 2 + emptyOffsets[0]) * xMult;
    basePoint[1] += (tickLength * 2 + emptyOffsets[1]) * yMult;

    // Create tick geometries. A tick is a square.
    for (int iTick = 0; iTick < nTicksSides[iSide / 2]; iTick++)
    {
      int tickBasePoint[2];
      tickBasePoint[0] = basePoint[0] + (tickLength * 2) * xMult * iTick;
      tickBasePoint[1] = basePoint[1] + (tickLength * 2) * yMult * iTick;

      ids[0] = pointCnt;
      borderPoints->SetPoint(pointCnt++, tickBasePoint[0], tickBasePoint[1], 0.);
      ids[1] = pointCnt;
      borderPoints->SetPoint(pointCnt++, tickBasePoint[0] + tickLength * xMult, tickBasePoint[1] + tickLength * yMult, 0.);
      ids[2] = pointCnt;
      borderPoints->SetPoint(pointCnt++, tickBasePoint[0] + tickLength * xMult - tickWidth * yMult, tickBasePoint[1] + tickWidth * xMult + tickLength * yMult, 0.);
      ids[3] = pointCnt;
      borderPoints->SetPoint(pointCnt++, tickBasePoint[0] - tickWidth * yMult, tickBasePoint[1] + tickWidth * xMult, 0.);
      borderCells->InsertNextCell(4, ids);
    }
  }
  this->BorderPolyData->SetPoints(borderPoints);
  this->BorderPolyData->SetPolys(borderCells);

  this->ComputedBorderViewportSize[0] = vSize[0];
  this->ComputedBorderViewportSize[1] = vSize[1];
}

//------------------------------------------------------------------------------
void vtkF3DDropZoneActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->TextActor->PrintSelf(os, indent.GetNextIndent());
}

//------------------------------------------------------------------------------
vtkTextProperty* vtkF3DDropZoneActor::GetTextProperty()
{
  return this->TextMapper->GetTextProperty();
}
