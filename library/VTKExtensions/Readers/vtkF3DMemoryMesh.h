/**
 * @class vtkF3DMemoryMesh
 * @brief create vtkPolyData from STL vectors.
 *
 * Simple source which convert and copy STL vectors provided by the user
 * to internal structure of vtkPolyData.
 * Only supports triangular mesh.
 * Does not support point data (normals, tcoords...) nor cell data yet.
 */
#ifndef vtkF3DMemoryMesh_h
#define vtkF3DMemoryMesh_h

#include "vtkPolyDataAlgorithm.h"

class vtkF3DMemoryMesh : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DMemoryMesh* New();
  vtkTypeMacro(vtkF3DMemoryMesh, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Set contiguous list of positions.
   * Length of the list must be a multiple of 3.
   */
  void SetPoints(const std::vector<float>& positions);

  /**
   * Set contiguous list of triangles by vertex indices.
   * Length of the list must be a multiple of 3.
   */
  void SetTriangles(const std::vector<unsigned int>& triangles);

protected:
  vtkF3DMemoryMesh();
  ~vtkF3DMemoryMesh() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DMemoryMesh(const vtkF3DMemoryMesh&) = delete;
  void operator=(const vtkF3DMemoryMesh&) = delete;

  vtkNew<vtkPolyData> Mesh;
};

#endif
