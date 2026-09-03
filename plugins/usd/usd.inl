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
    catch (std::invalid_argument& e)
    {
      vtkWarningWithObjectMacro(importer, "Invalid subdivision level: " << subdivisionLevelStr << " (" << e.what() << ")");
    }
    catch (std::out_of_range& e)
    {
      vtkWarningWithObjectMacro(importer, "Subdivision level out of range: " << subdivisionLevelStr << " (" << e.what() << ")");
    }
  }
}
// clang-format on
