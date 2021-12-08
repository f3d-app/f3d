/**
 * @class   F3DOpenCascadeReader
 * @brief   The OpenCascade reader class
 *
 */

#ifndef F3DOpenCascadeReader_h
#define F3DOpenCascadeReader_h

#include "F3DReader.h"

#ifndef F3D_NO_VTK
#include "vtkF3DOCCTReader.h"

#include <vtksys/SystemTools.hxx>
#endif

class F3DOpenCascadeReader : public F3DReader
{
public:
  F3DOpenCascadeReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "OpenCascadeReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "OpenCascade files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".stp", ".step", ".igs", ".iges" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "model/iges", "application/vnd.step" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkF3DOCCTReader> reader = vtkSmartPointer<vtkF3DOCCTReader>::New();
    reader->SetFileName(fileName.c_str());
    reader->RelativeDeflectionOn();
    reader->SetLinearDeflection(0.1);
    reader->SetAngularDeflection(0.5);
    reader->ReadWireOn();
    std::string ext = vtksys::SystemTools::GetFilenameLastExtension(fileName);
    ext = vtksys::SystemTools::LowerCase(ext);
    using ff = vtkF3DOCCTReader::FILE_FORMAT;
    reader->SetFileFormat((ext == ".stp" || ext == ".step") ? ff::STEP : ff::IGES);
    return reader;
  }
#endif
};

#endif
