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
  void SetPoints(const std::vector<float>& positions, size_t timeIdx = 0);

  /**
   * Set contiguous list of normals.
   * Length of the list must be a multiple of 3 (or left empty).
   * Must match the number of points specified in SetPoints.
   * The list is copied internally.
   * The list can be empty.
   */
  void SetNormals(const std::vector<float>& normals, size_t timeIdx = 0);

  /**
   * Set contiguous list of texture coordinates.
   * Length of the list must be a multiple of 2 (or left empty).
   * Must match the number of points specified in SetPoints.
   * The list is copied internally.
   * The list can be empty.
   */
  void SetTCoords(const std::vector<float>& tcoords, size_t timeIdx = 0);

  /**
   * Set faces by vertex indices.
   * faceSizes contains the size of each face (3 is triangle, 4 is quad, etc...)
   * cellIndices is a contiguous array of all face indices
   * The length of faceIndices should be the sum of all values in faceSizes
   * The lists are copied internally.
   * The lists can be empty, resulting in a point cloud.
   */
  void SetFaces(const std::vector<unsigned int>& faceSizes,
    const std::vector<unsigned int>& faceIndices, size_t timeIdx = 0);

  /**
   * Set time range in seconds for the saved frames, [0,0] by default, no animation.
   */
  void SetTimeRange(std::pair<double, double> newTimeRange);

  /**
   * Set time step in seconds for the saved frames, 1 by default.
   */
  void SetTimeStep(double newTimeStep);

  /** Get number of animation frames.ß
   * result * timeStep is a total animation length in seconds.
   */
  size_t GetTemporalStateCount();

  /**
   * Resize underlying vector to store temporal sequence.
   * By default vector is of size one and acts like mesh without animation, it's a must to resize
   * the vector for animation sequence. newSize is the resulting size of the resised vector, in
   * other words number of animation frames.
   */
  void ReserveTemporalEntries(size_t newSize);

protected:
  vtkF3DMemoryMesh();
  ~vtkF3DMemoryMesh() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DMemoryMesh(const vtkF3DMemoryMesh&) = delete;
  void operator=(const vtkF3DMemoryMesh&) = delete;

  std::vector<vtkNew<vtkPolyData>> Meshes;
  std::pair<double, double> timeRange;
  double timeStep;
};

#endif
