/**
 * @class vtkF3DFaceVaryingPointDispatcher
 * @brief dispatch face-varying attributes by duplicating points.
 *
 * This filter processes arrays on point data in case some of them are flagged
 * as face-varying in which case points must be duplicated before rendering
 *
 */
#ifndef vtkF3DFaceVaryingPointDispatcher_h
#define vtkF3DFaceVaryingPointDispatcher_h

#include "vtkPolyDataAlgorithm.h"

class vtkF3DFaceVaryingPointDispatcher : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DFaceVaryingPointDispatcher* New();
  vtkTypeMacro(vtkF3DFaceVaryingPointDispatcher, vtkPolyDataAlgorithm);

  /**
   * Metadata used to indicate which interpolation is used on arrays
   */
  static vtkInformationIntegerKey* INTERPOLATION_TYPE();

protected:
  vtkF3DFaceVaryingPointDispatcher();
  ~vtkF3DFaceVaryingPointDispatcher() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DFaceVaryingPointDispatcher(const vtkF3DFaceVaryingPointDispatcher&) = delete;
  void operator=(const vtkF3DFaceVaryingPointDispatcher&) = delete;
};

#endif
