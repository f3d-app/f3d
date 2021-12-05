/**
 * @class   F3DAssimpReader
 * @brief   The Assimp reader class
 *
 */

#ifndef F3DAssimpReader_h
#define F3DAssimpReader_h

#include "F3DReader.h"

#ifndef F3D_NO_VTK
#include "vtkF3DAssimpImporter.h"
#endif

class F3DAssimpReader : public F3DReader
{
public:
  F3DAssimpReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "Assimp"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "Assimp files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".fbx", ".dae", ".dxf", ".off" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  virtual const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "application/vnd.fbx", "application/vnd.dae", "image/vnd.dxf", "application/vnd.off" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the scene reader (VTK importer) for the given filename
   */
  vtkSmartPointer<vtkImporter> CreateSceneReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkF3DAssimpImporter> reader = vtkSmartPointer<vtkF3DAssimpImporter>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }
#endif
};

#endif
