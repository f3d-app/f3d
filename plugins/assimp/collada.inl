void applyCustomImporter(vtkImporter* importer, const std::string& vtkNotUsed(fileName)) const override
{
  vtkF3DAssimpImporter* objImporter = vtkF3DAssimpImporter::SafeDownCast(importer);
 
  // Needed because of https://github.com/assimp/assimp/issues/4949
  objImporter->SetColladaFixup(true);
}
