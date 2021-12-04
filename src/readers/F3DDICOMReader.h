/**
 * @class   F3DDICOMReader
 * @brief   The DICOM reader class
 *
 */

#ifndef F3DDICOMReader_h
#define F3DDICOMReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkDICOMImageReader.h>
#endif

class F3DDICOMReader : public F3DReader
{
public:
  F3DDICOMReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "DICOMReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "DICOM images files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".dcm" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  virtual const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "application/dicom" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
