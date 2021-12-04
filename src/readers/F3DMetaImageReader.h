/**
 * @class   F3DMetaImageReader
 * @brief   The MetaImage reader class
 *
 */

#ifndef F3DMetaImageReader_h
#define F3DMetaImageReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkMetaImageReader.h>
#endif

  class F3DMetaImageReader : public F3DReader
{
public:
  F3DMetaImageReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "MetaImageReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "MetaImage files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".mha", ".mhd" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  virtual const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "application/vnd.mhd" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkMetaImageReader> reader = vtkSmartPointer<vtkMetaImageReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
