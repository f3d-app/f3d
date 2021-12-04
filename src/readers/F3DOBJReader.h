/**
 * @class   F3DOBJReader
 * @brief   The OBJ reader class
 *
 */

#ifndef F3DOBJReader_h
#define F3DOBJReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkOBJImporter.h>
#include <vtkOBJReader.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>
#endif

class F3DOBJReader : public F3DReader
{
public:
  F3DOBJReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "OBJReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "OBJ files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".obj" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  virtual const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "model/obj" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName(fileName.c_str());
    return reader;
  }

  /*
   * Create the scene reader (VTK importer) for the given filename
   */
  vtkSmartPointer<vtkImporter> CreateSceneReader(const std::string& fileName) const override
  {
    vtkNew<vtkOBJImporter> importer;
    importer->SetFileName(fileName.c_str());

    std::string path = vtksys::SystemTools::GetFilenamePath(fileName);
    importer->SetTexturePath(path.c_str());

#if VTK_VERSION_NUMBER <= VTK_VERSION_CHECK(9, 0, 20201129)
      // This logic is partially implemented in the OBJ importer itself
      // This has been backported in VTK 9.1
    std::string mtlFile = fileName + ".mtl";
    if (vtksys::SystemTools::FileExists(mtlFile))
    {
      importer->SetFileNameMTL(mtlFile.c_str());
      }
      else
      {
        mtlFile = path + "/" + vtksys::SystemTools::GetFilenameWithoutLastExtension(fileName) + ".mtl";
        if (vtksys::SystemTools::FileExists(mtlFile))
        {
          importer->SetFileNameMTL(mtlFile.c_str());
        }
      }
#endif
    return importer;
  }
#endif
};

#endif
