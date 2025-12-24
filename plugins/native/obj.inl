void applyCustomImporter(vtkImporter* importer, const std::string& fileName, vtkResourceStream* stream) const override
{
  vtkOBJImporter* objImporter = vtkOBJImporter::SafeDownCast(importer);

  // TODO fileName
  std::string path = vtksys::SystemTools::GetFilenamePath(fileName);
  objImporter->SetTexturePath(path.c_str());
}
