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

  for (vtkIdType i = 0; i < numFaces; i++)
  {
    inPolys->GetCellAtId(i, cellSize, cellPoints);
    Point3 sum = { 0.0, 0.0, 0.0 };
    for (vtkIdType j = 0; j < cellSize; j++)
    {
      double p[3];
      inPoints->GetPoint(cellPoints[j], p);
      sum = sum + Point3{ p[0], p[1], p[2] };
    }
    facePoints[i] = sum / static_cast<double>(cellSize);
  }

  // gather edges and their adjacent faces
  std::map<EdgeKey, EdgeInfo> edges;
  vtkIdType numEdges = 0;

  for (vtkIdType i = 0; i < numFaces; i++)
  {
    inPolys->GetCellAtId(i, cellSize, cellPoints);
    for (vtkIdType j = 0; j < cellSize; j++)
    {
      vtkIdType v0 = cellPoints[j];
      vtkIdType v1 = cellPoints[(j + 1) % cellSize];
      EdgeInfo& info = edges[MakeEdgeKey(v0, v1)];
      if (info.PointId < 0)
      {
        info.PointId = numPts + numEdges++;
      }
      if (info.NumFaces < 2)
      {
        info.FacePoints[info.NumFaces] = i;
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
    Point3 midpoint = (v0 + v1) / 2.0;
    bool isBoundary = info.NumFaces < 2;

    info.Point = isBoundary
      ? midpoint
      : (v0 + v1 + facePoints[info.FacePoints[0]] + facePoints[info.FacePoints[1]]) / 4.0;

    for (vtkIdType v : key)
    {
      edgeAcc[v] = edgeAcc[v] + midpoint;
      edgeCount[v]++;
      if (isBoundary)
      {
        boundaryAcc[v] = boundaryAcc[v] + midpoint;
        boundaryCount[v]++;
      }
    }
  }

  for (vtkIdType i = 0; i < numFaces; i++)
  {
    inPolys->GetCellAtId(i, cellSize, cellPoints);
    for (vtkIdType j = 0; j < cellSize; j++)
    {
      vtkIdType v = cellPoints[j];
      faceAcc[v] = faceAcc[v] + facePoints[i];
      faceCount[v]++;
    }
  }

  // new position of each original vertex
  vtkNew<vtkPoints> outPoints;
  vtkIdType facePointOffset = numPts + numEdges;
  outPoints->SetNumberOfPoints(numPts + numEdges + numFaces);

  for (vtkIdType v = 0; v < numPts; v++)
  {
    double p[3];
    inPoints->GetPoint(v, p);
    Point3 original = { p[0], p[1], p[2] };

    Point3 newPoint;
    if (boundaryCount[v] > 0)
    {
      // boundary rule
      assert(boundaryCount[v] == 2);
      newPoint = (original * 6.0 + boundaryAcc[v]) / 8.0;
    }
    else
    {
      // interior rule
      double n = faceCount[v];
      Point3 F = faceAcc[v] / n;
      Point3 R = edgeAcc[v] / edgeCount[v];
      newPoint = (F + R * 2.0 + original * (n - 3.0)) / n;
    }
    outPoints->SetPoint(v, newPoint.data());
  }

  for (const auto& [key, info] : edges)
  {
    outPoints->SetPoint(info.PointId, info.Point.data());
  }

  for (vtkIdType f = 0; f < numFaces; f++)
  {
    outPoints->SetPoint(facePointOffset + f, facePoints[f].data());
  }

  // new topology: one quad per original cell corner
  vtkNew<vtkCellArray> outPolys;

  for (vtkIdType i = 0; i < numFaces; i++)
  {
    inPolys->GetCellAtId(i, cellSize, cellPoints);
    for (vtkIdType j = 0; j < cellSize; j++)
    {
      vtkIdType vPrev = cellPoints[(j + cellSize - 1) % cellSize];
      vtkIdType v = cellPoints[j];
      vtkIdType vNext = cellPoints[(j + 1) % cellSize];

      vtkIdType ePrev = edges[MakeEdgeKey(vPrev, v)].PointId;
      vtkIdType eNext = edges[MakeEdgeKey(v, vNext)].PointId;

      vtkIdType quad[4] = { v, eNext, facePointOffset + i, ePrev };
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
