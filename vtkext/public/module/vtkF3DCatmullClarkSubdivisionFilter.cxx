#include "vtkF3DCatmullClarkSubdivisionFilter.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

#include <array>
#include <map>
#include <vector>

vtkStandardNewMacro(vtkF3DCatmullClarkSubdivisionFilter);

namespace
{
// 3D point helpers
using Point3 = std::array<double, 3>;

Point3 operator+(const Point3& a, const Point3& b)
{
  return { a[0] + b[0], a[1] + b[1], a[2] + b[2] };
}

Point3 operator/(const Point3& a, double s)
{
  return { a[0] / s, a[1] / s, a[2] / s };
}

Point3 operator*(const Point3& a, double s)
{
  return { a[0] * s, a[1] * s, a[2] * s };
}

// Edge key for a pair of vertex ids, sorted to ensure uniqueness
using EdgeKey = std::array<vtkIdType, 2>;

EdgeKey MakeEdgeKey(vtkIdType a, vtkIdType b)
{
  return a < b ? EdgeKey{ a, b } : EdgeKey{ b, a };
}

// One edge of the input mesh, at most 2 adjacent faces since the mesh is manifold
struct EdgeInfo
{
  vtkIdType FacePoints[2] = { -1, -1 };
  vtkIdType NumFaces = 0;
  vtkIdType PointId = -1;
  Point3 Point = { 0.0, 0.0, 0.0 };
};

// Perform a single step of Catmull-Clark subdivision
// Returns false if the input mesh is not manifold and cannot be subdivided
bool SubdivideOnce(vtkPolyData* input, vtkPolyData* output)
{
  vtkPoints* inPoints = input->GetPoints();
  vtkCellArray* inPolys = input->GetPolys();
  vtkIdType numPts = inPoints->GetNumberOfPoints();
  vtkIdType numFaces = inPolys->GetNumberOfCells();

  vtkIdType cellSize = -1;
  vtkIdType const* cellPoints = nullptr;

  // face point: average of the vertices of each face
  std::vector<Point3> facePoints(numFaces, Point3{ 0.0, 0.0, 0.0 });

  for (vtkIdType faceId = 0; faceId < numFaces; faceId++)
  {
    inPolys->GetCellAtId(faceId, cellSize, cellPoints);
    Point3 sum = { 0.0, 0.0, 0.0 };
    for (vtkIdType vertexId = 0; vertexId < cellSize; vertexId++)
    {
      double pt[3];
      inPoints->GetPoint(cellPoints[vertexId], pt);
      sum = sum + Point3{ pt[0], pt[1], pt[2] };
    }
    facePoints[faceId] = sum / static_cast<double>(cellSize);
  }

  // gather edges and their adjacent faces
  std::map<EdgeKey, EdgeInfo> edges;
  vtkIdType numEdges = 0;

  for (vtkIdType faceId = 0; faceId < numFaces; faceId++)
  {
    inPolys->GetCellAtId(faceId, cellSize, cellPoints);
    for (vtkIdType vertexId = 0; vertexId < cellSize; vertexId++)
    {
      vtkIdType v0 = cellPoints[vertexId];
      vtkIdType v1 = cellPoints[(vertexId + 1) % cellSize];
      EdgeInfo& info = edges[MakeEdgeKey(v0, v1)];
      if (info.PointId < 0)
      {
        info.PointId = numPts + numEdges++;
      }

      if (info.NumFaces < 2)
      {
        info.FacePoints[info.NumFaces] = faceId;
        info.NumFaces++;
      }
      else
      {
        return false;
      }
    }
  }

  // per-vertex accumulators used to compute the new vertex position rule
  std::vector<Point3> faceAcc(numPts, Point3{ 0.0, 0.0, 0.0 });
  std::vector<int> faceCount(numPts, 0);
  std::vector<Point3> edgeAcc(numPts, Point3{ 0.0, 0.0, 0.0 });
  std::vector<int> edgeCount(numPts, 0);
  std::vector<Point3> boundaryAcc(numPts, Point3{ 0.0, 0.0, 0.0 });
  std::vector<int> boundaryCount(numPts, 0);

  for (auto& [key, info] : edges)
  {
    double p0[3], p1[3];
    inPoints->GetPoint(key[0], p0);
    inPoints->GetPoint(key[1], p1);
    Point3 v0 = { p0[0], p0[1], p0[2] };
    Point3 v1 = { p1[0], p1[1], p1[2] };
    Point3 midPoint = (v0 + v1) / 2.0;
    bool isBoundary = info.NumFaces < 2;

    info.Point = isBoundary
      ? midPoint
      : (v0 + v1 + facePoints[info.FacePoints[0]] + facePoints[info.FacePoints[1]]) / 4.0;

    for (vtkIdType pointId : key)
    {
      edgeAcc[pointId] = edgeAcc[pointId] + midPoint;
      edgeCount[pointId]++;
      if (isBoundary)
      {
        boundaryAcc[pointId] = boundaryAcc[pointId] + midPoint;
        boundaryCount[pointId]++;
      }
    }
  }

  for (vtkIdType faceId = 0; faceId < numFaces; faceId++)
  {
    inPolys->GetCellAtId(faceId, cellSize, cellPoints);
    for (vtkIdType vertexId = 0; vertexId < cellSize; vertexId++)
    {
      vtkIdType pointId = cellPoints[vertexId];
      faceAcc[pointId] = faceAcc[pointId] + facePoints[faceId];
      faceCount[pointId]++;
    }
  }

  // new position of each original vertex
  vtkNew<vtkPoints> outPoints;
  vtkIdType facePointOffset = numPts + numEdges;
  outPoints->SetNumberOfPoints(numPts + numEdges + numFaces);

  for (vtkIdType pointId = 0; pointId < numPts; pointId++)
  {
    double pt[3];
    inPoints->GetPoint(pointId, pt);
    Point3 original = { pt[0], pt[1], pt[2] };

    Point3 newPoint;
    if (boundaryCount[pointId] > 0)
    {
      // boundary rule
      assert(boundaryCount[pointId] == 2);
      newPoint = (original * 6.0 + boundaryAcc[pointId]) / 8.0;
    }
    else
    {
      // interior rule
      double n = faceCount[pointId];
      Point3 F = faceAcc[pointId] / n;
      Point3 R = edgeAcc[pointId] / edgeCount[pointId];
      newPoint = (F + R * 2.0 + original * (n - 3.0)) / n;
    }
    outPoints->SetPoint(pointId, newPoint.data());
  }

  for (const auto& [key, info] : edges)
  {
    outPoints->SetPoint(info.PointId, info.Point.data());
  }

  for (vtkIdType faceId = 0; faceId < numFaces; faceId++)
  {
    outPoints->SetPoint(facePointOffset + faceId, facePoints[faceId].data());
  }

  // new topology: one quad per original cell corner
  vtkNew<vtkCellArray> outPolys;

  for (vtkIdType faceId = 0; faceId < numFaces; faceId++)
  {
    inPolys->GetCellAtId(faceId, cellSize, cellPoints);
    for (vtkIdType vertexId = 0; vertexId < cellSize; vertexId++)
    {
      vtkIdType vPrev = cellPoints[(vertexId + cellSize - 1) % cellSize];
      vtkIdType vCurrent = cellPoints[vertexId];
      vtkIdType vNext = cellPoints[(vertexId + 1) % cellSize];

      vtkIdType ePrev = edges[MakeEdgeKey(vPrev, vCurrent)].PointId;
      vtkIdType eNext = edges[MakeEdgeKey(vCurrent, vNext)].PointId;

      vtkIdType quad[4] = { vCurrent, eNext, facePointOffset + faceId, ePrev };
      outPolys->InsertNextCell(4, quad);
    }
  }

  output->SetPoints(outPoints);
  output->SetPolys(outPolys);

  return true;
}
}

//------------------------------------------------------------------------------
vtkF3DCatmullClarkSubdivisionFilter::vtkF3DCatmullClarkSubdivisionFilter() = default;

//------------------------------------------------------------------------------
vtkF3DCatmullClarkSubdivisionFilter::~vtkF3DCatmullClarkSubdivisionFilter() = default;

//------------------------------------------------------------------------------
int vtkF3DCatmullClarkSubdivisionFilter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0]->GetInformationObject(0));
  vtkPolyData* output = vtkPolyData::GetData(outputVector->GetInformationObject(0));

  vtkSmartPointer<vtkPolyData> current = vtkSmartPointer<vtkPolyData>::New();
  current->ShallowCopy(input);

  for (int i = 0; i < this->NumberOfSubdivisions; i++)
  {
    vtkNew<vtkPolyData> next;
    if (!::SubdivideOnce(current, next))
    {
      vtkErrorMacro("Subdivision failed due to non-manifold geometry");
      return 0;
    }
    current = next;
  }

  output->ShallowCopy(current);

  return 1;
}
