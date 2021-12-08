/**
 * @class   F3DJPEGReader
 * @brief   The JPEG reader class
 *
 */

#ifndef F3DJPEGReader_h
#define F3DJPEGReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkJPEGReader.h>
#endif

class F3DJPEGReader : public F3DReader
{
public:
  F3DJPEGReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "JPEGReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "JPEG files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".jpg", ".jpeg" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "image/jpeg" };
    return types;
  }

  /*
   * Get the data dimension
   */
  int DataDimension() const override { return 2; }

  /*
   * Get the data type
   */
  int DataType() const override  { return DATA_TYPE_IMAGE; }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkJPEGReader> reader = vtkSmartPointer<vtkJPEGReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
