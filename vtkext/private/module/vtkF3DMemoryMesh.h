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
   * Set the time range of the animated mesh.
   * Calling this is optional, in which case the mesh will be considered static.
   * It must be called before the algorithm is updated.
   */
  void SetTimeRange(double startTime, double endTime);

  /**
   * Set the update function to call when the pipeline updates.
   * The function is passed the time for which the update is triggered and a pointer to the
   * vtkPolyData to update. The function must fill the provided vtkPolyData with the mesh data
   * corresponding to the provided time. The function is called at least once when the algorithm is
   * updated, and can be called multiple times if the pipeline requests updates at different times.
   * The function can be changed at any time, but it must be set before the algorithm is updated.
   */
  void SetUpdateFunction(std::function<void(double, vtkPolyData*)> updateFunction);

protected:
  vtkF3DMemoryMesh();
  ~vtkF3DMemoryMesh() override;

  int RequestInformation(vtkInformation* vtkNotUsed(request),
    vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DMemoryMesh(const vtkF3DMemoryMesh&) = delete;
  void operator=(const vtkF3DMemoryMesh&) = delete;

  vtkNew<vtkPolyData> Mesh;
  double TimeRange[2] = { 0.0, 0.0 };
  std::function<void(double, vtkPolyData*)> UpdateFunction;
};

#endif
