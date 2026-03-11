void applyCustomImporter(vtkImporter* importer, const std::string& vtkNotUsed(fileName)) const override
{
  vtkF3DAssimpImporter* assimpImporter = vtkF3DAssimpImporter::SafeDownCast(importer);
 
  // Needed because of https://github.com/assimp/assimp/issues/4949
  assimpImporter->SetColladaFixup(true);
}
