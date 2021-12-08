/**
 * @class   F3DPNGReader
 * @brief   The PNG reader class
 *
 */

#ifndef F3DPNGReader_h
#define F3DPNGReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkPNGReader.h>
#endif

class F3DPNGReader : public F3DReader
{
public:
  F3DPNGReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "PNGReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "PNG files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".png" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "image/png" };
    return types;
  }

  /*
   * Get the data dimension
   */
  virtual int DataDimension() const { return 2; }

  /*
   * Get the data type
   */
  virtual int DataType() const { return DATA_TYPE_IMAGE; }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
