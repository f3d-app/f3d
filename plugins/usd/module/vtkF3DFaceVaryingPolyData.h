/**
 * @class vtkF3DFaceVaryingPolyData
 * @brief dispatch face-varying attributes by duplicating points.
 *
 * TODO
 */
#ifndef vtkF3DFaceVaryingPolyData_h
#define vtkF3DFaceVaryingPolyData_h

#include "vtkPolyDataAlgorithm.h"

class vtkF3DFaceVaryingPolyData : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DFaceVaryingPolyData* New();
  vtkTypeMacro(vtkF3DFaceVaryingPolyData, vtkPolyDataAlgorithm);

  vtkSetMacro(NormalsFaceVarying, bool);
  vtkSetMacro(TCoordsFaceVarying, bool);

protected:
  vtkF3DFaceVaryingPolyData();
  ~vtkF3DFaceVaryingPolyData() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DFaceVaryingPolyData(const vtkF3DFaceVaryingPolyData&) = delete;
  void operator=(const vtkF3DFaceVaryingPolyData&) = delete;

  bool NormalsFaceVarying = false;
  bool TCoordsFaceVarying = false;
};

#endif
