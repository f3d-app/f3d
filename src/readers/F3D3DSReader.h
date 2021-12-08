/**
 * @class   F3D3DSReader
 * @brief   The 3DS reader class
 *
 */

#ifndef F3D3DSReader_h
#define F3D3DSReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtk3DSImporter.h>
#endif

class F3D3DSReader : public F3DReader
{
public:
  F3D3DSReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "3DSReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "3DS files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".3ds" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "application/vnd.3ds" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the scene reader (VTK importer) for the given filename
   */
  vtkSmartPointer<vtkImporter> CreateSceneReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtk3DSImporter> importer = vtkSmartPointer<vtk3DSImporter>::New();
    importer->SetFileName(fileName.c_str());
    return importer;
  }
#endif
};

#endif
