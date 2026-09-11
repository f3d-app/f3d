/**
 * @class   F3DOCCTPolyData
 * @brief   Namespace converting a meshed OCCT shape into a vtkPolyData
 *
 * Shared by the OCCT based readers: the shape is meshed with the provided
 * deflections, then faces and optionally edges are converted into polys and
 * lines carrying normals, texture coordinates and, when color providers are
 * given, RGBA cell colors.
 */

#ifndef F3DOCCTPolyData_h
#define F3DOCCTPolyData_h

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include <array>
#include <functional>

namespace F3DOCCTPolyData
{
using Color = std::array<unsigned char, 4>;

struct MeshingOptions
{
  double LinearDeflection = 0.1;
  double AngularDeflection = 0.5;
  bool RelativeDeflection = false;
  bool ReadWire = false;
  bool ReadFaces = true;
};

/**
 * Color callbacks, both are optional. The Colors cell array is only created
 * when at least one of them is set, missing faces are white and missing edges
 * are black.
 */
struct ColorProviders
{
  std::function<Color(const TopoDS_Face&)> Face;
  std::function<Color(const TopoDS_Edge&)> Edge;
};

/**
 * Mesh the shape and convert it into a polydata.
 * Point coordinates and normals are expressed in the shape location.
 */
vtkSmartPointer<vtkPolyData> Create(
  const TopoDS_Shape& shape, const MeshingOptions& options, const ColorProviders& colors = {});
}

#endif
