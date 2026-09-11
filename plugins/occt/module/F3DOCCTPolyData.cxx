#include "F3DOCCTPolyData.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Poly.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_Triangulation.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <gp_Mat.hxx>
#include <gp_Trsf.hxx>

#include <numeric>
#include <vector>

namespace F3DOCCTPolyData
{
//----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> Create(
  const TopoDS_Shape& shape, const MeshingOptions& options, const ColorProviders& colors)
{
  vtkNew<vtkPoints> points;
  points->SetDataTypeToDouble();
  vtkNew<vtkFloatArray> normals;
  normals->SetNumberOfComponents(3);
  normals->SetName("Normal");
  vtkNew<vtkFloatArray> uvs;
  uvs->SetNumberOfComponents(2);
  uvs->SetName("UV");
  vtkNew<vtkUnsignedCharArray> cellColors;
  cellColors->SetNumberOfComponents(4);
  cellColors->SetName("Colors");
  const bool hasColors = colors.Face || colors.Edge;
  vtkNew<vtkCellArray> triangles;
  vtkNew<vtkCellArray> lines;

  vtkIdType shift = 0;

  /* Mesh the whole shape. This only affect faces, edges have to be handled separately. */
  BRepMesh_IncrementalMesh(
    shape, options.LinearDeflection, options.RelativeDeflection, options.AngularDeflection, true);

  if (options.ReadWire)
  {
    std::vector<TopoDS_Edge> edges;
    {
      /* add all edges to a compound to remesh them all at once */
      TopoDS_Builder builder;
      TopoDS_Compound compound;
      builder.MakeCompound(compound);
      for (TopExp_Explorer exEdge(shape, TopAbs_EDGE); exEdge.More(); exEdge.Next())
      {
        const TopoDS_Edge edge = TopoDS::Edge(exEdge.Current());
        builder.Add(compound, edge);
        edges.push_back(edge);
      }
      BRepMesh_IncrementalMesh(compound, options.LinearDeflection, options.RelativeDeflection,
        options.AngularDeflection, true);
    }

    for (const TopoDS_Edge& edge : edges)
    {
      TopLoc_Location location;
      const auto& poly = BRep_Tool::Polygon3D(edge, location);
      if (poly.IsNull())
      {
        continue;
      }

      const int nbV = poly->NbNodes();
      const NCollection_Array1<gp_Pnt>& nodes = poly->Nodes();
      for (int i = 1; i <= nbV; i++)
      {
        const gp_Pnt pt = nodes(i).Transformed(location);
        points->InsertNextPoint(pt.X(), pt.Y(), pt.Z());

        /* normals and uvs make no sense for lines */
        const float fn[3] = { 0.0, 0.0, 1.0 };
        normals->InsertNextTypedTuple(fn);
        uvs->InsertNextTypedTuple(fn);
      }

      std::vector<vtkIdType> polyline(nbV);
      std::iota(polyline.begin(), polyline.end(), shift);
      lines->InsertNextCell(static_cast<vtkIdType>(polyline.size()), polyline.data());

      if (hasColors)
      {
        const Color rgba = colors.Edge ? colors.Edge(edge) : Color{ 0, 0, 0, 255 };
        cellColors->InsertNextTypedTuple(rgba.data());
      }

      shift += nbV;
    }
  }

  for (TopExp_Explorer exFace(shape, TopAbs_FACE); options.ReadFaces && exFace.More();
       exFace.Next())
  {
    const TopoDS_Face face = TopoDS::Face(exFace.Current());
    TopLoc_Location location;
    const auto& poly = BRep_Tool::Triangulation(face, location);
    if (poly.IsNull())
    {
      continue;
    }

    Poly::ComputeNormals(poly);
    const TopAbs_Orientation faceOrientation = face.Orientation();
    const gp_Trsf& trsf = location.Transformation();
    /* normals follow the location like the points, but a mirroring location
     * flips the triangle winding, hence the normals derived from it */
    const float normalSign = (trsf.VectorialPart().Determinant() < 0.0) !=
        (faceOrientation == TopAbs_Orientation::TopAbs_REVERSED)
      ? -1.f
      : 1.f;
    const int nbT = poly->NbTriangles();
    const int nbV = poly->NbNodes();

    for (int i = 1; i <= nbV; i++)
    {
      const gp_Pnt pt = poly->Node(i).Transformed(location);
      points->InsertNextPoint(pt.X(), pt.Y(), pt.Z());
    }

    if (poly->HasNormals())
    {
      for (int i = 1; i <= nbV; i++)
      {
        const gp_Dir n = poly->Normal(i).Transformed(trsf);
        const float fn[3] = { normalSign * static_cast<float>(n.X()),
          normalSign * static_cast<float>(n.Y()), normalSign * static_cast<float>(n.Z()) };
        normals->InsertNextTypedTuple(fn);
      }
    }
    else
    {
      /* just in case a face does not have normals, add a dummy normal */
      const float fn[3] = { 0.0, 0.0, 1.0 };
      for (int i = 1; i <= nbV; i++)
      {
        normals->InsertNextTypedTuple(fn);
      }
    }

    if (poly->HasUVNodes())
    {
      for (int i = 1; i <= nbV; i++)
      {
        const gp_Pnt2d uv = poly->UVNode(i);
        const float fuv[2] = { static_cast<float>(uv.X()), static_cast<float>(uv.Y()) };
        uvs->InsertNextTypedTuple(fuv);
      }
    }
    else
    {
      const float fuv[2] = { 0.f, 0.f };
      for (int i = 1; i <= nbV; i++)
      {
        uvs->InsertNextTypedTuple(fuv);
      }
    }

    const Color rgba = colors.Face ? colors.Face(face) : Color{ 255, 255, 255, 255 };
    for (int i = 1; i <= nbT; i++)
    {
      int n1, n2, n3;
      poly->Triangle(i).Get(n1, n2, n3);
      vtkIdType cell[3] = { shift + n1 - 1, shift + n2 - 1, shift + n3 - 1 };
      if (faceOrientation != TopAbs_Orientation::TopAbs_FORWARD)
      {
        std::swap(cell[0], cell[2]);
      }
      triangles->InsertNextCell(3, cell);
      if (hasColors)
      {
        cellColors->InsertNextTypedTuple(rgba.data());
      }
    }

    shift += nbV;
  }

  vtkNew<vtkPolyData> polydata;
  polydata->SetPoints(points);
  polydata->GetPointData()->SetNormals(normals);
  polydata->GetPointData()->SetTCoords(uvs);
  polydata->SetLines(lines);
  polydata->SetPolys(triangles);
  if (hasColors)
  {
    polydata->GetCellData()->SetScalars(cellColors);
  }
  polydata->Squeeze();
  return polydata;
}
}
