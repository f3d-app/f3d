/**
 * @class   vtkF3DPSDReader
 * @brief   Read a PSD image file
 */
#ifndef vtkF3DPSDReader_h
#define vtkF3DPSDReader_h

#include "vtkImageReader.h"
#include "vtkVersion.h"

class vtkResourceStream;

class vtkF3DPSDReader : public vtkImageReader
{
public:
  static vtkF3DPSDReader* New();
  vtkTypeMacro(vtkF3DPSDReader, vtkImageReader);

  ///@{
  /**
   * Return 1 if, after a quick check of file header, it looks like the provided stream
   * can be read. Return 0 if it is sure it cannot be read as a stream.
   *
   * Only check the header contains "8BPS"
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
    return ".psd";
  }

  /**
   * Descriptive name of the reader
   */
  const char* GetDescriptiveName() override
  {
    return "PSD";
  }

protected:
  vtkF3DPSDReader();
  ~vtkF3DPSDReader() override;

  void ExecuteInformation() override;
  void ExecuteDataWithInformation(vtkDataObject* out, vtkInformation* outInfo) override;

private:
  vtkF3DPSDReader(const vtkF3DPSDReader&) = delete;
  void operator=(const vtkF3DPSDReader&) = delete;

  std::vector<unsigned char> BufferData;
};

#endif
