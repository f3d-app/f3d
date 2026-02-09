void applyCustomImporter(vtkImporter* importer, const std::string& vtkNotUsed(fileName), vtkResourceStream* stream) const override
{ 
  if (stream)
  {
    // Recover hint
    std::string hint;
    vtkF3DAssimpImporter::CanReadFile(stream, hint);

    // Set hint
    vtkF3DAssimpImporter* assimpImporter = vtkF3DAssimpImporter::SafeDownCast(importer);
    assimpImporter->SetMemoryHint(hint);
  }
}
