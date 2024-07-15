void applyCustomImporter(vtkImporter* importer, const std::string& fileName) const override
{
  vtkOBJImporter* objImporter = vtkOBJImporter::SafeDownCast(importer);

  std::string path = vtksys::SystemTools::GetFilenamePath(fileName);
  objImporter->SetTexturePath(path.c_str());

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
}
