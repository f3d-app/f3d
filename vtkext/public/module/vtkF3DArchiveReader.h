/**
 * @class   vtkF3DArchiveReader
 * @brief   Read entries from a zip archive
 *
 * Provide random access to the entries of a zip archive stored in a seekable
 * vtkResourceStream, so that readers of archive based formats can extract the
 * files they need without unpacking the whole archive.
 */

#ifndef vtkF3DArchiveReader_h
#define vtkF3DArchiveReader_h

#include "vtkextModule.h"

/// @cond
#include <vtkObject.h>

#include <memory>
#include <string>
#include <vector>
/// @endcond

class vtkResourceStream;

class VTKEXT_EXPORT vtkF3DArchiveReader : public vtkObject
{
public:
  static vtkF3DArchiveReader* New();
  vtkTypeMacro(vtkF3DArchiveReader, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Open the zip archive contained in the provided stream and read its index.
   * The stream must support seeking and is kept referenced until the archive is
   * closed or this reader is destroyed.
   * Return false if the stream is not seekable or does not contain a valid archive.
   */
  bool Open(vtkResourceStream* stream);

  /**
   * Close the currently opened archive, if any, and release the stream.
   */
  void Close();

  /**
   * Return true if an entry with the provided name exists in the opened archive.
   * Names are the full paths of the entries, eg: "dir/file.txt".
   */
  bool Has(const std::string& name);

  /**
   * Extract the uncompressed content of the named entry into out.
   * Return false if the archive is not opened, if the entry does not exist or if
   * its content cannot be extracted.
   */
  bool Extract(const std::string& name, std::vector<char>& out);

protected:
  vtkF3DArchiveReader();
  ~vtkF3DArchiveReader() override;

private:
  vtkF3DArchiveReader(const vtkF3DArchiveReader&) = delete;
  void operator=(const vtkF3DArchiveReader&) = delete;

  class vtkInternals;
  std::unique_ptr<vtkInternals> Internals;
};

#endif
