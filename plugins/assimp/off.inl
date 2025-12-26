void applyCustomImporter(vtkImporter* importer, const std::string& vtkNotUsed(fileName), vtkResourceStream*) const override
{
  vtkF3DAssimpImporter* assimpImporter = vtkF3DAssimpImporter::SafeDownCast(importer);
  assimpImporter->SetMemoryHint("off");
}
