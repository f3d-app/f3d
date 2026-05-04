/**
 * @class   vtkF3DOpen3SDCMReader
 * @brief   VTK Reader for 3Shape DCM files
 *
 * This reader uses the Open3SDCM library to parse 3Shape DCM files
 * (Dental CAD Model format) and convert them to VTK polydata.
 * 
 * 3Shape DCM files are ZIP archives containing HPS (Himsa Packed Scan) XML
 * with base64-encoded binary vertex and facet data.
 * 
 * Supported schemas: CA, CB, CC (unencrypted), CE (encrypted with Blowfish)
 * 
 * @warning This reader requires the Open3SDCM library and its dependencies
 * (Poco, OpenSSL) to be available at build time.
 */

#ifndef vtkF3DOpen3SDCMReader_h
#define vtkF3DOpen3SDCMReader_h

#include <vtkPolyDataAlgorithm.h>

class vtkResourceStream;

class vtkF3DOpen3SDCMReader : public vtkPolyDataAlgorithm
{
public:
  /**
   * Instantiate this class.
   */
  static vtkF3DOpen3SDCMReader* New();

  /**
   * Print the class name and version.
   */
  vtkTypeMacro(vtkF3DOpen3SDCMReader, vtkPolyDataAlgorithm);

  /**
   * Set the file name to read from.
   */
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  /**
   * Set the resource stream to read from (for in-memory data).
   * Note: Stream reading is currently not supported as Open3SDCM requires file paths.
   */
  vtkSetObjectMacro(ResourceStream);
  vtkGetObjectMacro(ResourceStream, vtkResourceStream);

  /**
   * Check if the reader can read the given file.
   * Returns true if the file is a valid 3Shape DCM file.
   */
  static bool CanReadFile(const std::string& filename);

  /**
   * Check if the reader can read the given stream.
   * Currently always returns false as stream reading is not supported.
   */
  static bool CanReadStream(vtkResourceStream* stream);

protected:
  vtkF3DOpen3SDCMReader();
  ~vtkF3DOpen3SDCMReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  char* FileName;
  vtkResourceStream* ResourceStream;

private:
  vtkF3DOpen3SDCMReader(const vtkF3DOpen3SDCMReader&) = delete;
  void operator=(const vtkF3DOpen3SDCMReader&) = delete;
};

#endif // vtkF3DOpen3SDCMReader_h
