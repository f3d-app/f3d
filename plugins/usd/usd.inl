// clang-format off
void applyCustomImporter(
  vtkImporter* importer, const std::string& vtkNotUsed(fileName), vtkResourceStream*) const override
{
  vtkF3DUSDImporter* usdImporter = vtkF3DUSDImporter::SafeDownCast(importer);
  usdImporter->SetResourcesPath(this->ReaderOptions.at("USD.resources_path"));
}
// clang-format on
