bool canRead(vtkResourceStream* stream) const override
{
  std::string hint;
  bool ret = vtkF3DAssimpImporter::CanReadFile(stream, hint);
  // clang-format off
  ret &= (hint == "dae");
  // clang-format on
  return ret;
}

void applyCustomImporter(vtkImporter* importer, const std::string& vtkNotUsed(fileName),
  vtkResourceStream* stream) const override
{
  vtkF3DAssimpImporter* assimpImporter = vtkF3DAssimpImporter::SafeDownCast(importer);

  // Needed because of https://github.com/assimp/assimp/issues/4949
  assimpImporter->SetColladaFixup(true);

  if (stream)
  {
    // clang-format off
    assimpImporter->SetMemoryHint("dae");
    // clang-format on
  }
}
