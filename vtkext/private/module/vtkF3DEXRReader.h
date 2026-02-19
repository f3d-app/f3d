#ifndef vtkF3DEXRReader_h
#define vtkF3DEXRReader_h

#include "vtkImageReader.h"
#include "vtkVersion.h"

class vtkResourceStream;

class vtkF3DEXRReader : public vtkImageReader
{
public:
  static vtkF3DEXRReader* New();
  vtkTypeMacro(vtkF3DEXRReader, vtkImageReader);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Return 1 if, after a quick check of file header, it looks like the provided stream
   * can be read. Return 0 if it is sure it cannot be read as a stream.
   *
   * Only check the header contains the Imf::MAGIC
   */
  int CanReadFile(const char* fname) override;
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260106)
  int CanReadFile(vtkResourceStream* stream) override;
#else
  int CanReadFile(vtkResourceStream* stream);
#endif
  ///@}

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
