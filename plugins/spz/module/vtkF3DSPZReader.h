/**
 * @class   vtkF3DSPZReader
 * @brief   Reader using Niantics SPZ library
 *
 * TODO
 *
 * @sa https://github.com/nianticlabs/spz/blob/main/README.md
 *
 */

#ifndef vtkF3DSPZReader_h
#define vtkF3DSPZReader_h

#include <vtkNew.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkVersion.h>

#include <memory>

class vtkF3DSPZReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DSPZReader* New();
  vtkTypeMacro(vtkF3DSPZReader, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Set the file name.
   */
  vtkSetMacro(FileName, std::string);

protected:
  vtkF3DSPZReader();
  ~vtkF3DSPZReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  std::string FileName;

private:
  vtkF3DSPZReader(const vtkF3DSPZReader&) = delete;
  void operator=(const vtkF3DSPZReader&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif
