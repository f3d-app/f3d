/**
 * @class vtkF3DFaceVaryingPointDispatcher
 * @brief dispatch face-varying attributes by duplicating points.
 *
 * This filter processes arrays on point data in case some of them are flagged
 * as face-varying in which case points must be duplicated before rendering
 *
 * Face-varying attributes are a special case between point and cell data where the number of tuples
 * in the attributes is equal to the cell connectivity array size.
 * For example, if we have two adjacent quads, we will have 6 points and 8 cell indices (4 per quad)
 * Face-varying attributes, even if located on point data will have 8 tuples, and not 6
 * It can be seen as attributes, but this filter will normalize it by outputting 8 points.
 *
 */
#ifndef vtkF3DFaceVaryingPointDispatcher_h
#define vtkF3DFaceVaryingPointDispatcher_h

#include <vtkPolyDataAlgorithm.h>

#include "vtkextModule.h"

class VTKEXT_EXPORT vtkF3DFaceVaryingPointDispatcher : public vtkPolyDataAlgorithm
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
