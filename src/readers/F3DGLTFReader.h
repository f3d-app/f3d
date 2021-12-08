/**
 * @class   F3DGLTFReader
 * @brief   The glTF reader class
 *
 */

#ifndef F3DGLTFReader_h
#define F3DGLTFReader_h

#include "F3DReaderFactory.h"

#ifndef F3D_NO_VTK
#include <vtkGLTFReader.h>
#include <vtkGLTFImporter.h>
#endif

class F3DGLTFReader : public F3DReader
{
public:
  F3DGLTFReader() = default;

  /*
   * Get the name of this reader
   */
  const std::string GetName() const override { return "glTFReader"; }

  /*
   * Get the short description of this reader
   */
  const std::string GetShortDescription() const override { return "glTF files reader"; }

  /*
   * Get the extensions supported by this reader
   */
  const std::vector<std::string> GetExtensions() const override
  {
    static const std::vector<std::string> ext = { ".gltf", ".glb" };
    return ext;
  }

  /*
   * Get the mimetypes supported by this reader
   */
  const std::vector<std::string> GetMimeTypes() const override
  {
    static const std::vector<std::string> types = { "model/gltf-binary", "model/gltf+json" };
    return types;
  }

#ifndef F3D_NO_VTK
  /*
   * Create the geometry reader (VTK reader) for the given filename
   */
  vtkSmartPointer<vtkAlgorithm> CreateGeometryReader(const std::string& fileName) const override
  {
    vtkSmartPointer<vtkGLTFReader> reader = vtkSmartPointer<vtkGLTFReader>::New();
    reader->SetFileName(fileName.c_str());

    // Enable all animations in the GLTFReader
    reader->SetFrameRate(30);
    reader->ApplyDeformationsToGeometryOn();
    reader->UpdateInformation(); // Read model metadata to get the number of animations
    for (vtkIdType i = 0; i < reader->GetNumberOfAnimations(); i++)
    {
      reader->EnableAnimation(i);
    }
    // It is needed to update the information directly in order to recover it later
    // Not entirely understood, TODO
    reader->UpdateInformation();

    return reader;
  }

  /*
   * Create the scene reader (VTK importer) for the given filename
   */
  vtkSmartPointer<vtkImporter> CreateSceneReader(const std::string& fileName) const override
  {
    vtkNew<vtkGLTFImporter> importer;
    importer->SetFileName(fileName.c_str());
    return importer;
  }
#endif
};

#endif
