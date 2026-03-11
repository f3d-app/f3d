void applyCustomImporter(vtkImporter* importer, const std::string& fileName) const override
{
  vtkOBJImporter* objImporter = vtkOBJImporter::SafeDownCast(importer);

  std::string path = vtksys::SystemTools::GetFilenamePath(fileName);
  objImporter->SetTexturePath(path.c_str());
}
