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
   * Set cells by vertex indices.
   * cellSize contains the size of each cell (3 is triangle, 4 is quad, etc...)
   * cellIndices is a contiguous array of all cells indices
   * The length of cellIndices should be the sum of all values in cellSize
   * The lists are copied internally.
   */
  void SetCells(
    const std::vector<unsigned int>& cellSize, const std::vector<unsigned int>& cellIndices);

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
