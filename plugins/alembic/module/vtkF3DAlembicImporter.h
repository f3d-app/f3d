/**
 * @class   vtkF3DAlembicImporter
 * @brief   Importer using Alembic library
 *
 * This importer is based on Alembic 1.7
 * Currently, only polygonal points positions are retrieved
 * to build polygonal geometries. Vertex normals and texture
 * coordinates are not supported yet.
 *
 * @sa https://github.com/alembic/alembic/blob/master/README.txt
 *
 */

#ifndef vtkF3DAlembicImporter_h
#define vtkF3DAlembicImporter_h

#include <vtkImporter.h>
#include <vtkNew.h>
#include <vtkVersion.h>

#include <memory>

class vtkF3DAlembicImporter : public vtkImporter
{
public:
  static vtkF3DAlembicImporter* New();
  vtkTypeMacro(vtkF3DAlembicImporter, vtkImporter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Set the file name.
   */
  vtkSetMacro(FileName, std::string);

protected:
  vtkF3DAlembicImporter();
  ~vtkF3DAlembicImporter() override;

  int ImportBegin() override;
  void ImportActors(vtkRenderer*) override;

  std::string FileName;

private:
  vtkF3DAlembicImporter(const vtkF3DAlembicImporter&) = delete;
  void operator=(const vtkF3DAlembicImporter&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif
