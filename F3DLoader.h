/**
 * @class   F3DLoader
 * @brief   The loader class
 *
 */

#ifndef vtkF3DLoader_h
#define vtkF3DLoader_h

#include "Config.h"

#include "F3DOptions.h"

#include <vtkImporter.h>
#include <vtkSmartPointer.h>

class vtkImporter;

class F3DLoader
{
public:
  F3DLoader(const F3DOptions& options) : Options(options) {}

  vtkSmartPointer<vtkImporter> GetImporter() const;

protected:
  const F3DOptions& Options;
};

#endif
