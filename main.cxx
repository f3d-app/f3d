#include <cstdio>
#include <iostream>

#include "F3DViewer.h"
#include "F3DOptions.h"

#include "vtkF3DGenericImporter.h"

#include <vtkOBJImporter.h>
#include <vtk3DSImporter.h>
#include <vtkVRMLImporter.h>
#include <vtkNew.h>
#include <vtksys/SystemTools.hxx>

#if VTK_VERSION_MAJOR == 8 && VTK_VERSION_MINOR > 2
#include <vtkGLTFImporter.h>
#endif

int main(int argc, char **argv)
{
  F3DOptions options;
  options.InitializeFromArgs(argc, argv);

  if (!vtksys::SystemTools::FileExists(options.Input))
  {
    cerr << "Specified input file '" << options.Input << "' does not exists." << endl;
    return -1;
  }

  vtkSmartPointer<vtkImporter> importer;
  bool importerFound = false;
  std::string fileName = options.Input.c_str();

  if (options.Importer)
  {
    std::string ext = vtksys::SystemTools::GetFilenameLastExtension(fileName);
    ext = vtksys::SystemTools::LowerCase(ext);

    if (!importerFound && ext == ".3ds")
    {
      vtk3DSImporter* localImporter = vtk3DSImporter::New();
      localImporter->SetFileName(fileName.c_str());
      localImporter->SetComputeNormals(options.Normals);
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
        mtlFile = path + "/" + vtksys::SystemTools::GetFilenameWithoutLastExtension(fileName) + ".mtl";
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

#if VTK_VERSION_MAJOR == 8 && VTK_VERSION_MINOR > 2
    if (!importerFound && (ext == ".gltf" || ext == ".glb"))
    {
      vtkGLTFImporter* localImporter = vtkGLTFImporter::New();
      localImporter->SetFileName(fileName.c_str());
      importer.TakeReference(localImporter);
      importerFound = true;
    }
#endif
  }

  if (!importerFound)
  {
    vtkF3DGenericImporter* localImporter = vtkF3DGenericImporter::New();
    localImporter->SetFileName(fileName.c_str());
    localImporter->SetOptions(options);
    importer.TakeReference(localImporter);
  }

  F3DViewer viewer(&options, importer);
  return viewer.Start();
}
