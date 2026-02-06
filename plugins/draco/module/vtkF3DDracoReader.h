/**
 * @class   vtkF3DDracoReader
 * @brief   VTK Reader for DRC files using Draco
 *
 * This reader is based on Draco 1.5
 * It supports meshes and point clouds encoded by Draco in .drc files
 * It reads properly normals, texture coordinates, and colors.
 * All other attributes are added to the point data as generic arrays.
 *
 * This reader supports reading streams.
 *
 */

#ifndef vtkF3DDracoReader_h
#define vtkF3DDracoReader_h

#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>

#include <memory>

class vtkResourceStream;
class vtkF3DDracoReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DDracoReader* New();
  vtkTypeMacro(vtkF3DDracoReader, vtkPolyDataAlgorithm);

  /**
   * Specify stream to read from
   * When both `Stream` and `Filename` are set, stream is used.
   */
  void SetStream(vtkResourceStream* stream);

  ///@{
  /**
   * Get/Set the file name.
   */
  vtkSetMacro(FileName, std::string);
  vtkGetMacro(FileName, std::string);
  ///@}

  /**
   * Return true if, after a quick check of file header, it looks like the provided stream
   * can be read. Return false if it is sure it cannot be read as a strean.
   *
   * This only checks that the first 5 bytes spells "DRACO".
   */
  static bool CanReadFile(vtkResourceStream* stream);

protected:
  vtkF3DDracoReader();
  ~vtkF3DDracoReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DDracoReader(const vtkF3DDracoReader&) = delete;
  void operator=(const vtkF3DDracoReader&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;

  vtkSmartPointer<vtkResourceStream> Stream;
  std::string FileName;
};

#endif
