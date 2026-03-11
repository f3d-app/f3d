/**
 * @class   vtkF3DPDALReader
 * @brief   Specialized version of the PDAL reader to read direct scalars data
 *
 * A specialized version of the vtkPDALReader that is able to read colors to be displayed
 * as direct scalars
 */

#ifndef vtkF3DPDALReader_h
#define vtkF3DPDALReader_h

#include <vtkPDALReader.h>

class vtkF3DPDALReader : public vtkPDALReader
{
public:
  static vtkF3DPDALReader* New();
  vtkTypeMacro(vtkF3DPDALReader, vtkPDALReader);

protected:
  vtkF3DPDALReader() = default;
  ~vtkF3DPDALReader() override = default;

  /**
   * Call request data on the superclass, then, if Colors are provided,
   * convert them into a normalized vtkDoubleArray for direct scalars rendering.
   */
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DPDALReader(const vtkF3DPDALReader&) = delete;
  void operator=(const vtkF3DPDALReader&) = delete;
};

#endif
