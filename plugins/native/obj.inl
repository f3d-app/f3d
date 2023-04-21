void applyCustomImporter(vtkImporter* importer, const std::string& fileName) const override
{
  vtkOBJImporter* objImporter = vtkOBJImporter::SafeDownCast(importer);

  std::string path = vtksys::SystemTools::GetFilenamePath(fileName);
  objImporter->SetTexturePath(path.c_str());

// This logic needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7447
#if VTK_VERSION_NUMBER <= VTK_VERSION_CHECK(9, 0, 20201129)
  // This logic is partially implemented in the OBJ importer itself
  // This has been backported in VTK 9.1
  std::string mtlFile = fileName + ".mtl";
  if (vtksys::SystemTools::FileExists(mtlFile))
  {
    objImporter->SetFileNameMTL(mtlFile.c_str());
  }
  else
  {
    mtlFile = path + "/" + vtksys::SystemTools::GetFilenameWithoutLastExtension(fileName) + ".mtl";
    if (vtksys::SystemTools::FileExists(mtlFile))
    {
      objImporter->SetFileNameMTL(mtlFile.c_str());
    }
  }
#endif
}
