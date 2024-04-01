#ifndef vtkF3DEXRReader_h
#define vtkF3DEXRReader_h

#include "vtkImageReader.h"

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
};

#endif
