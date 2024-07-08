/**
 * @class   vtkF3DImporter
 * @brief   Generic importer for F3D
 *
 * This generic importer is provided to simplify implementation of other importers
 * and handle multiple versions of VTK.
 */

#ifndef vtkF3DImporter_h
#define vtkF3DImporter_h

#include "vtkextModule.h"

#include <vtkImporter.h>
#include <vtkVersion.h>

class VTKEXT_EXPORT vtkF3DImporter : public vtkImporter
{
public:
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240707)
  /**
   * This method should be reimplemented in importer
   * implementations to handle update the importer at a specific time value
   * then call this method and return what it returns.
   */
  bool UpdateAtTimeValue(double timeValue) override;
#else  
  virtual bool UpdateAtTimeValue(double timeValue);
  void UpdateTimeStep(double timeValue) override;
#endif

  /**
   * Call this method to set the status to failure if supported
   * by the VTK version in use
   */
  void SetFailureStatus();
};

#endif
