/**
 * @class vtkF3DMemoryMesh
 * @brief create vtkPolyData from vectors.
 *
 * Simple source which convert and copy vectors provided by the user
 * to internal structure of vtkPolyData.
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

  /**
   * Set contiguous list of positions.
   * Length of the list must be a multiple of 3.
   * The list is copied internally.
   */
  void SetPoints(const std::vector<float>& positions);

  /**
   * Set contiguous list of normals.
   * Length of the list must be a multiple of 3 (or left empty).
   * Must match the number of points specified in SetPoints.
   * The list is copied internally.
   * The list can be empty.
   */
  void SetNormals(const std::vector<float>& normals);

  /**
   * Set contiguous list of texture coordinates.
   * Length of the list must be a multiple of 2 (or left empty).
   * Must match the number of points specified in SetPoints.
   * The list is copied internally.
   * The list can be empty.
   */
  void SetTCoords(const std::vector<float>& tcoords);

  /**
   * Set faces by vertex indices.
   * faceSizes contains the size of each face (3 is triangle, 4 is quad, etc...)
   * cellIndices is a contiguous array of all face indices
   * The length of faceIndices should be the sum of all values in faceSizes
   * The lists are copied internally.
   * The lists can be empty, resulting in a point cloud.
   */
  void SetFaces(
    const std::vector<unsigned int>& faceSizes, const std::vector<unsigned int>& faceIndices);

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
