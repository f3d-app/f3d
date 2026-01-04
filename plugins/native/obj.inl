void applyCustomImporter(vtkImporter* importer, const std::string& fileName, vtkResourceStream*) const override
{
  vtkOBJImporter* objImporter = vtkOBJImporter::SafeDownCast(importer);

  if (!fileName.empty())
  {
    std::string path = vtksys::SystemTools::GetFilenamePath(fileName);
    objImporter->SetTexturePath(path.c_str());
  }
}
