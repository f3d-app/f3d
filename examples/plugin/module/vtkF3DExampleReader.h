#ifndef vtkF3DExampleReader_h
#define vtkF3DExampleReader_h

#include <vtkPolyDataAlgorithm.h>

/**
 * @class   vtkF3DExampleReader
 * @brief   Simple Example reader
 *
 * This reader is a very simple reader which illustrate how to create
 * a new VTK reader to integrate it into F3D.
 * It reads a text file (extension must be ".expl") which contains a point cloud.
 * Point coordinates are separated by spaces, tabs, or new lines.
 */
class vtkF3DExampleReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DExampleReader* New();
  vtkTypeMacro(vtkF3DExampleReader, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Get/Set the file name.
   */
  vtkSetMacro(FileName, std::string);
  vtkGetMacro(FileName, std::string);
  ///@}

protected:
  vtkF3DExampleReader();
  ~vtkF3DExampleReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DExampleReader(const vtkF3DExampleReader&) = delete;
  void operator=(const vtkF3DExampleReader&) = delete;

  std::string FileName;
};

#endif
