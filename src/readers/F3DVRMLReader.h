/**
 * @class   F3DVRMLReader
 * @brief   The VRML reader class
 *
 */

#ifndef F3DVRMLReader_h
#define F3DVRMLReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkVRMLImporter.h>
#endif

class F3DVRMLReader : public F3DReader
{
public:
  F3DVRMLReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "VRMLReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "VRML files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".wrl", ".vrml" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  virtual const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "model/vrml" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the scene reader (VTK importer) for the given filename
   */
  vtkSmartPointer<vtkImporter> CreateSceneReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkVRMLImporter> importer = vtkSmartPointer<vtkVRMLImporter>::New();
    importer->SetFileName(fileName.c_str());
    return importer;
  }
#endif
};

#endif
