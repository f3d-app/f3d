#ifndef vtkF3DWebPReader_h
#define vtkF3DWebPReader_h

#include "vtkImageReader.h"

class vtkF3DWebPReader : public vtkImageReader
{
public:
  static vtkF3DWebPReader* New();
  vtkTypeMacro(vtkF3DWebPReader, vtkImageReader);
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
    return ".webp";
  }

  /**
   * Descriptive name of the reader
   */
  const char* GetDescriptiveName() override
  {
    return "WebP";
  }

protected:
  vtkF3DWebPReader();
  ~vtkF3DWebPReader() override;

  void ExecuteInformation() override;
  void ExecuteDataWithInformation(vtkDataObject* out, vtkInformation* outInfo) override;

  int GetWidth() const;
  int GetHeight() const;

private:
  vtkF3DWebPReader(const vtkF3DWebPReader&) = delete;
  void operator=(const vtkF3DWebPReader&) = delete;

  std::vector<unsigned char> BufferData;
};

#endif
