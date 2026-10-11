/**
 * @class vtkF3DCatmullClarkSubdivisionFilter
 * @brief Subdivides polygonal data using the Catmull-Clark subdivision algorithm.
 *
 * Implementation based on https://en.wikipedia.org/wiki/Catmull%E2%80%93Clark_subdivision_surface
 */
#ifndef vtkF3DCatmullClarkSubdivisionFilter_h
#define vtkF3DCatmullClarkSubdivisionFilter_h

/// @cond
#include <vtkSubdivisionFilter.h>
/// @endcond

#include "vtkextModule.h"

class VTKEXT_EXPORT vtkF3DCatmullClarkSubdivisionFilter : public vtkSubdivisionFilter
{
public:
  static vtkF3DCatmullClarkSubdivisionFilter* New();
  vtkTypeMacro(vtkF3DCatmullClarkSubdivisionFilter, vtkSubdivisionFilter);

protected:
  vtkF3DCatmullClarkSubdivisionFilter();
  ~vtkF3DCatmullClarkSubdivisionFilter() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DCatmullClarkSubdivisionFilter(const vtkF3DCatmullClarkSubdivisionFilter&) = delete;
  void operator=(const vtkF3DCatmullClarkSubdivisionFilter&) = delete;
};

#endif
