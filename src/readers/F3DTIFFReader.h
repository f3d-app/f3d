/**
 * @class   F3DTIFFReader
 * @brief   The TIFF reader class
 *
 */

#ifndef F3DTIFFReader_h
#define F3DTIFFReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkTIFFReader.h>
#endif

class F3DTIFFReader : public F3DReader
{
public:
  F3DTIFFReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "TIFFReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "TIFF files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".tif", ".tiff" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "application/x-tgif" };
    return types;
  }

  /*
   * Get the data type
   */
  virtual int GetDataType() const { return DATA_TYPE_IMAGE; }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
