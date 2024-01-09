/**
 * @class   vtkF3DAlembicReader
 * @brief   Reader using Alembic library
 *
 * This reader is based on Alembic 1.7
 * Currently, only polygonal points positions are retrieved
 * to build polygonal geometries. Vertex normals and texture
 * coordinates are not supported yet.
 *
 * @sa https://github.com/alembic/alembic/blob/master/README.txt
 *
 */

#ifndef vtkF3DAlembicReader_h
#define vtkF3DAlembicReader_h

#include <vtkNew.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkVersion.h>

#include <memory>

class vtkF3DAlembicReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DAlembicReader* New();
  vtkTypeMacro(vtkF3DAlembicReader, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Set the file name.
   */
  vtkSetMacro(FileName, std::string);

protected:
  vtkF3DAlembicReader();
  ~vtkF3DAlembicReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  std::string FileName;

private:
  vtkF3DAlembicReader(const vtkF3DAlembicReader&) = delete;
  void operator=(const vtkF3DAlembicReader&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif
