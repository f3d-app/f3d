#ifndef vtkF3DEXRReader_h
#define vtkF3DEXRReader_h

#include "vtkImageReader.h"

#include <OpenEXR/ImfIO.h>

class vtkF3DEXRReader : public vtkImageReader
{
public:
  static vtkF3DEXRReader* New();
  vtkTypeMacro(vtkF3DEXRReader, vtkImageReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /**
   * Return 1 if the reader is compatible with the given file
   */
  int CanReadFile(const char* fname) override;

  /**
   * List of extensions supported by this reader
   */
  const char* GetFileExtensions() override
  {
    return ".exr";
  }

  /**
   * Descriptive name of the reader
   */
  const char* GetDescriptiveName() override
  {
    return "OpenEXR";
  }

  /**
   * Read from memory instead of file
   */
  void SetMemoryBuffer(const void* buff) override;

  /**
   * Specify the in memory image buffer length.
   */
  void SetMemoryBufferLength(vtkIdType buflen) override;

protected:
  vtkF3DEXRReader();
  ~vtkF3DEXRReader() override;

  void ExecuteInformation() override;
  void ExecuteDataWithInformation(vtkDataObject* out, vtkInformation* outInfo) override;

  int GetWidth() const;
  int GetHeight() const;

private:
  vtkF3DEXRReader(const vtkF3DEXRReader&) = delete;
  void operator=(const vtkF3DEXRReader&) = delete;

  /**
   * Class to treat file contents in memory like it were still in a file.
   */
  class MemStream : public Imf::IStream
  {
  public:
    MemStream(const char* name, const void* buff, vtkIdType bufferLen)
      : Imf::IStream(name)
      , buffer(static_cast<const char*>(buff))
      , bufflen(static_cast<size_t>(bufferLen))
      , pos(0)
    {
    }

    bool read(char c[], int n) override;

    /**
     * returns the current reading position, in bytes, from the beginning of the file.
     * The next read() call will begin reading at the indicated position
     */
    uint64_t tellg() override
    {
      return pos;
    }

    /**
     * sets the current reading position to pos bytes from the beginning of the "file"
     */
    void seekg(uint64_t new_pos) override
    {
      pos = new_pos;
    }

    /**
     * clears any error flags (we dont have to worry about this)
     */
    void clear() override
    {
    }

  private:
    const char* buffer;
    size_t bufflen;
    uint64_t pos;
  };
};

#endif
