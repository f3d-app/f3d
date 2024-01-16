/**
 * @class vtkF3DFaceVaryingPolyData
 * @brief dispatch face-varying attributes by duplicating points.
 *
 * This filter processes arrays on point data in case some of them are flagged
 * as face-varying in which case points must be duplicated before rendering
 *
 */
#ifndef vtkF3DFaceVaryingPolyData_h
#define vtkF3DFaceVaryingPolyData_h

#include "vtkPolyDataAlgorithm.h"

class vtkF3DFaceVaryingPolyData : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DFaceVaryingPolyData* New();
  vtkTypeMacro(vtkF3DFaceVaryingPolyData, vtkPolyDataAlgorithm);

  /**
   * Metadata used to indicate which interpolation is used on arrays
   */
  static vtkInformationIntegerKey* INTERPOLATION_TYPE();

protected:
  vtkF3DFaceVaryingPolyData();
  ~vtkF3DFaceVaryingPolyData() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DFaceVaryingPolyData(const vtkF3DFaceVaryingPolyData&) = delete;
  void operator=(const vtkF3DFaceVaryingPolyData&) = delete;
};

#endif
