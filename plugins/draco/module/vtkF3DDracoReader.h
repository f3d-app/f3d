/**
 * @class   vtkF3DDracoReader
 * @brief   VTK Reader for DRC files using Draco
 *
 * This reader is based on Draco 1.5
 * It supports meshes and point clouds encoded by Draco in .drc files
 * It reads properly normals, texture coordinates, and colors.
 * All other attributes are added to the point data as generic arrays.
 *
 */

#ifndef vtkF3DDracoReader_h
#define vtkF3DDracoReader_h

#include <vtkPolyDataAlgorithm.h>

#include <memory>

class vtkF3DDracoReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DDracoReader* New();
  vtkTypeMacro(vtkF3DDracoReader, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Get/Set the file name.
   */
  vtkSetMacro(FileName, std::string);
  vtkGetMacro(FileName, std::string);
  ///@}

protected:
  vtkF3DDracoReader();
  ~vtkF3DDracoReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DDracoReader(const vtkF3DDracoReader&) = delete;
  void operator=(const vtkF3DDracoReader&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;

  std::string FileName;
};

#endif
