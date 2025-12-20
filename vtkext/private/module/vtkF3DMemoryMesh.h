/**
 * @class vtkF3DMemoryMesh
 * @brief create vtkPolyData from vectors or a callback.
 *
 * Simple source which converts vectors provided by the user
 * to internal structure of vtkPolyData, or uses a callback  for animated mesh generation.
 */
#ifndef vtkF3DMemoryMesh_h
#define vtkF3DMemoryMesh_h

#include "vtkPolyDataAlgorithm.h"

#include <functional>

class vtkF3DMemoryMesh : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DMemoryMesh* New();
  vtkTypeMacro(vtkF3DMemoryMesh, vtkPolyDataAlgorithm);

  /**
   * Callback type for animated meshes.
   * Called with the current time value, should populate the mesh using the Set* methods.
   */
  using MeshCallback = std::function<void(double time, vtkF3DMemoryMesh* mesh)>;

  /**
   * Set contiguous list of positions for a static mesh.
   * Length of the list must be a multiple of 3.
   * The list is copied internally.
   */
  void SetPoints(const std::vector<float>& positions);

  /**
   * Set contiguous list of normals for a static mesh.
   * Length of the list must be a multiple of 3 (or left empty).
   * Must match the number of points specified in SetPoints.
   * The list is copied internally.
   * The list can be empty.
   */
  void SetNormals(const std::vector<float>& normals);

  /**
   * Set contiguous list of texture coordinates for a static mesh.
   * Length of the list must be a multiple of 2 (or left empty).
   * Must match the number of points specified in SetPoints.
   * The list is copied internally.
   * The list can be empty.
   */
  void SetTCoords(const std::vector<float>& tcoords);

  /**
   * Set faces by vertex indices for a static mesh.
   * faceSizes contains the size of each face (3 is triangle, 4 is quad, etc...)
   * cellIndices is a contiguous array of all face indices
   * The length of faceIndices should be the sum of all values in faceSizes
   * The lists are copied internally.
   * The lists can be empty, resulting in a point cloud.
   */
  void SetFaces(
    const std::vector<unsigned int>& faceSizes, const std::vector<unsigned int>& faceIndices);

  /**
   * Set a callback for animated mesh generation.
   * The callback is invoked at each requested time and should populate the mesh
   * using SetPoints, SetNormals, SetTCoords, and SetFaces methods.
   * This switches the mesh to animated mode with the given time range.
   */
  void SetAnimatedMesh(double startTime, double endTime, MeshCallback callback);

protected:
  vtkF3DMemoryMesh();
  ~vtkF3DMemoryMesh() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DMemoryMesh(const vtkF3DMemoryMesh&) = delete;
  void operator=(const vtkF3DMemoryMesh&) = delete;

  vtkNew<vtkPolyData> StaticMesh;

  MeshCallback AnimatedCallback;
  double TimeRange[2] = { 0.0, 0.0 };
  bool IsAnimated = false;
};

#endif
