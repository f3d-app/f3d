/**
 * @class   vtkF3DAlembicReader
 * @brief   Reader using Alembic library
 *
 * This reader is based on Alembic 1.7
 * Currently, only polygonal points positions are retrieved
 * to build polygonal geometries. Vertex normals and texture
 * coordinates are not supported yet.
 *
 * This reader supports reading streams.
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

class vtkResourceStream;
class vtkF3DAlembicReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DAlembicReader* New();
  vtkTypeMacro(vtkF3DAlembicReader, vtkPolyDataAlgorithm);

  /**
   * Set the file name.
   */
  vtkSetMacro(FileName, std::string);

  /**
   * Specify stream to read from
   * Only Ogawa version of alembic format is supported.
   * When both `Stream` and `Filename` are set, stream is used.
   */
  void SetStream(vtkResourceStream* stream);

  /**
   * Overridden to take into account mtime from the internal vtkResourceStream.
   */
  vtkMTimeType GetMTime() override;

  /**
   * Return true if, after a quick check of file header, it looks like the provided stream
   * can be read. Return false if it is sure it cannot be read as a strean.
   *
   * This only checks that the first 5 bytes spells "Ogawa".
   */
  static bool CanReadFile(vtkResourceStream* stream);

protected:
  vtkF3DAlembicReader();
  ~vtkF3DAlembicReader() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkF3DAlembicReader(const vtkF3DAlembicReader&) = delete;
  void operator=(const vtkF3DAlembicReader&) = delete;

  vtkSmartPointer<vtkResourceStream> Stream;
  std::string FileName;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif
