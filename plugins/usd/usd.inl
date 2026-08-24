// clang-format off
void applyCustomImporter(
  vtkImporter* importer, const std::string& vtkNotUsed(fileName), vtkResourceStream*) const override
{
  vtkF3DUSDImporter* usdImporter = vtkF3DUSDImporter::SafeDownCast(importer);
  usdImporter->SetResourcesPath(this->ReaderOptions.at("USD.resources_path"));

  std::string subdivisionLevelStr = this->ReaderOptions.at("USD.subdivision_level");
  if (!subdivisionLevelStr.empty())
  {
    try
    {
      usdImporter->SetSubdivisionLevel(std::stoi(subdivisionLevelStr));
    }
    catch (...)
    {
      vtkWarningWithObjectMacro(nullptr, "Invalid subdivision level: " << subdivisionLevelStr);
    }
  }
}
// clang-format on
