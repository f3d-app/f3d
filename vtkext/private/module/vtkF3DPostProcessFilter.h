/**
 * @class vtkF3DPostProcessFilter
 * @brief merges blocks and extract surface.
 *
 * F3D supports a range of data format, from meshes to 3D images.
 * In order to display each dataset properly, this filter post process them to different types.
 * This filter takes one input dataset and outputs three different dataset:
 *  1/ the surface (hull) of the dataset as a vtkPolyData
 *  2/ a point cloud of the dataset as a vtkPolyData
 *  3/ a 3D image sampling of the dataset as a volumic vtkImageData (if supported)
 */

#ifndef vtkF3DPostProcessFilter_h
#define vtkF3DPostProcessFilter_h

#include "vtkDataObjectAlgorithm.h"

class vtkF3DPostProcessFilter : public vtkDataObjectAlgorithm
{
public:
  static vtkF3DPostProcessFilter* New();
  vtkTypeMacro(vtkF3DPostProcessFilter, vtkDataObjectAlgorithm);

  vtkF3DPostProcessFilter(const vtkF3DPostProcessFilter&) = delete;
  void operator=(const vtkF3DPostProcessFilter&) = delete;

protected:
  vtkF3DPostProcessFilter();
  ~vtkF3DPostProcessFilter() override = default;

  int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int FillOutputPortInformation(int port, vtkInformation* info) override;
};

#endif
