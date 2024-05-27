/**
 * @class   vtkQuakeMDLReader
 * @brief   VTK Reader for Quake 1 models in binary .mdl file format
 */

#ifndef vtkQuakeMDLReader_h
#define vtkQuakeMDLReader_h

#include <vtkPolyDataAlgorithm.h>

class vtkQuakeMDLReader : public vtkPolyDataAlgorithm
{
public:
  static vtkQuakeMDLReader* New();
  vtkTypeMacro(vtkQuakeMDLReader, vtkPolyDataAlgorithm);

  /**
   * Set the file name.
   */
  vtkSetMacro(FileName, std::string);

protected:
  vtkQuakeMDLReader();
  ~vtkQuakeMDLReader() override = default;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkQuakeMDLReader(const vtkQuakeMDLReader&) = delete;
  void operator=(const vtkQuakeMDLReader&) = delete;

  std::string FileName;
};

#endif
