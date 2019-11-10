#include "F3DLoader.h"

#include "vtkF3DGenericImporter.h"

#include <vtk3DSImporter.h>
#include <vtkGLTFImporter.h>
#include <vtkNew.h>
#include <vtkOBJImporter.h>
#include <vtkVRMLImporter.h>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkSmartPointer<vtkImporter> F3DLoader::GetImporter() const
{
  if (!vtksys::SystemTools::FileExists(this->Options.Input))
  {
    cerr << "Specified input file '" << this->Options.Input << "' does not exist!" << endl;
    exit(EXIT_FAILURE);
  }

  vtkSmartPointer<vtkImporter> importer;
  bool importerFound = false;
  std::string fileName = this->Options.Input.c_str();

  if (this->Options.Importer)
  {
    std::string ext = vtksys::SystemTools::GetFilenameLastExtension(fileName);
    ext = vtksys::SystemTools::LowerCase(ext);

    if (!importerFound && ext == ".3ds")
    {
      vtk3DSImporter* localImporter = vtk3DSImporter::New();
      localImporter->SetFileName(fileName.c_str());
      localImporter->SetComputeNormals(this->Options.Normals);
      importer.TakeReference(localImporter);
      importerFound = true;
    }

    if (!importerFound && ext == ".obj")
    {
      vtkOBJImporter* localImporter = vtkOBJImporter::New();
      localImporter->SetFileName(fileName.c_str());

      std::string path = vtksys::SystemTools::GetFilenamePath(fileName);
      localImporter->SetTexturePath(path.c_str());

      // This logic is partially implemented in the OBJ importer itself
      // This complete version should be backported.
      std::string mtlFile = fileName + ".mtl";
      if (vtksys::SystemTools::FileExists(mtlFile))
      {
        localImporter->SetFileNameMTL(mtlFile.c_str());
      }
      else
      {
        mtlFile =
          path + "/" + vtksys::SystemTools::GetFilenameWithoutLastExtension(fileName) + ".mtl";
        if (vtksys::SystemTools::FileExists(mtlFile))
        {
          localImporter->SetFileNameMTL(mtlFile.c_str());
        }
      }

      importer.TakeReference(localImporter);
      importerFound = true;
    }

    if (!importerFound && ext == ".wrl")
    {
      vtkVRMLImporter* localImporter = vtkVRMLImporter::New();
      localImporter->SetFileName(fileName.c_str());
      importer.TakeReference(localImporter);
      importerFound = true;
    }

    if (!importerFound && (ext == ".gltf" || ext == ".glb"))
    {
      vtkGLTFImporter* localImporter = vtkGLTFImporter::New();
      localImporter->SetFileName(fileName.c_str());
      importer.TakeReference(localImporter);
      importerFound = true;
    }
  }

  if (!importerFound)
  {
    vtkF3DGenericImporter* localImporter = vtkF3DGenericImporter::New();
    localImporter->SetFileName(fileName.c_str());
    localImporter->SetOptions(this->Options);
    importer.TakeReference(localImporter);
  }

  return importer;
}
